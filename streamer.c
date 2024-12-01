#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "encoder.h"
#include <errno.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 1000

typedef struct {
    int socket_fd;
    int client_id;
    int active;
    int paused;
} ClientInfo;

pthread_mutex_t lock;

void *handle_client(void *arg) {
    ClientInfo *client = (ClientInfo *)arg;
    char buffer[BUFFER_SIZE];
    FILE *client_file = NULL;

    system("mkdir -p streams");

    char filename[50];
    snprintf(filename, sizeof(filename), "streams/cliente_%03d.txt", client->client_id);
    client_file = fopen(filename, "w");
    if (!client_file) {
        perror("Error al crear el archivo del cliente");
        close(client->socket_fd);
        free(client);
        return NULL;
    }

    printf("Cliente %03d conectado en el socket %d.\n", client->client_id, client->socket_fd);

    send(client->socket_fd, "Seleccione modo (HD, MD, LD): ", 30, 0);
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client->socket_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) {
        printf("Cliente %03d desconectado antes de seleccionar calidad.\n", client->client_id);
        fclose(client_file);
        close(client->socket_fd);
        free(client);
        return NULL;
    }

    buffer[bytes_received] = '\0';

    if (strcmp(buffer, "HD") == 0) {
        set_client_bitrate(client->client_id, HD);
    } else if (strcmp(buffer, "MD") == 0) {
        set_client_bitrate(client->client_id, MD);
    } else if (strcmp(buffer, "LD") == 0) {
        set_client_bitrate(client->client_id, LD);
    } else {
        printf("Cliente %03d envió un comando no válido. Desconectando...\n", client->client_id);
        fclose(client_file);
        close(client->socket_fd);
        free(client);
        return NULL;
    }

    printf("Cliente %03d seleccionó %s.\n", client->client_id,
           strcmp(buffer, "HD") == 0 ? "HD" : strcmp(buffer, "MD") == 0 ? "MD" : "LD");

    while (client->active) {
        if (!client->paused) {
            int frame = get_next_frame(client->client_id);

            if (frame == -1) {
                printf("Cliente %03d ha recibido todos los frames disponibles. Desconectando automáticamente...\n", client->client_id);
                break;
            }

            fprintf(client_file, "%d\n", frame);
            printf("Enviando frame \"%d\" al cliente %03d.\n", frame, client->client_id);
            char frame_str[32];
            sprintf(frame_str, "%d", frame);
            send(client->socket_fd, frame_str, strlen(frame_str), 0);

            usleep(500000);
        }

        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client->socket_fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';

            if (strcmp(buffer, "HD") == 0) {
                set_client_bitrate(client->client_id, HD);
                printf("Cliente %03d cambió a HD.\n", client->client_id);
            } else if (strcmp(buffer, "MD") == 0) {
                set_client_bitrate(client->client_id, MD);
                printf("Cliente %03d cambió a MD.\n", client->client_id);
            } else if (strcmp(buffer, "LD") == 0) {
                set_client_bitrate(client->client_id, LD);
                printf("Cliente %03d cambió a LD.\n", client->client_id);
            } else if (strcmp(buffer, "STOP") == 0) {
                printf("Cliente %03d detuvo la transmisión.\n", client->client_id);
                break;
            } else if (strcmp(buffer, "PAUSE") == 0) {
                client->paused = 1;
                printf("Cliente %03d pausó la transmisión.\n", client->client_id);
            } else if (strcmp(buffer, "PLAY") == 0) {
                client->paused = 0;
                printf("Cliente %03d reanudó la transmisión.\n", client->client_id);
            } else {
                printf("Cliente %03d envió un comando no válido: %s.\n", client->client_id, buffer);
            }
        }
    }

    fclose(client_file);
    close(client->socket_fd);
    free(client);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <puerto> <archivo de video>\n", argv[0]);
        return 1;
    }

    init_encoder(argv[2]);

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear el socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al enlazar el socket");
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("Error al escuchar");
        return 1;
    }

    printf("Servidor escuchando en el puerto %s.\n", argv[1]);

    pthread_mutex_init(&lock, NULL);

    srand(time(NULL));

    while (1) {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Error al aceptar conexión");
            continue;
        }

        pthread_mutex_lock(&lock);
        int client_id = rand() % MAX_CLIENTS;
        pthread_mutex_unlock(&lock);

        ClientInfo *client = (ClientInfo *)malloc(sizeof(ClientInfo));
        client->socket_fd = client_fd;
        client->client_id = client_id;
        client->active = 1;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, client);
        pthread_detach(thread_id);
    }

    pthread_mutex_destroy(&lock);
    close(server_fd);
    free_encoder();
    return 0;
}
