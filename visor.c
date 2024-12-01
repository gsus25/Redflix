#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

void *receive_frames(void *arg) {
    int socket_fd = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("Conexión cerrada por el servidor. Finalizando cliente.\n");
            close(socket_fd);
            exit(0);
        }
        buffer[bytes_received] = '\0';
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <IP del servidor> <puerto>\n", argv[0]);
        return 1;
    }

    int socket_fd;
    struct sockaddr_in server_addr;
    char command[BUFFER_SIZE];
    pthread_t thread_id;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear el socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Dirección IP inválida");
        close(socket_fd);
        return 1;
    }

    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar con el servidor");
        close(socket_fd);
        return 1;
    }

    printf("Conectado al servidor.\n");

    if (pthread_create(&thread_id, NULL, receive_frames, &socket_fd) != 0) {
        perror("Error al crear el hilo de recepción");
        close(socket_fd);
        return 1;
    }

    while (1) {
        printf("Ingrese un comando (HD, MD, LD, STOP, PAUSE, PLAY): ");
        fgets(command, BUFFER_SIZE, stdin);
        command[strcspn(command, "\n")] = '\0';

        if (send(socket_fd, command, strlen(command), 0) < 0) {
            perror("Error al enviar comando");
            break;
        }

        if (strcmp(command, "STOP") == 0) {
            printf("Finalizando cliente.\n");
            break;
        }
    }

    pthread_cancel(thread_id);
    pthread_join(thread_id, NULL);
    close(socket_fd);

    return 0;
}