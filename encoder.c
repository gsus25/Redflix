#include "encoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CLIENTS 1000

static int *frames = NULL;
static int total_frames = 0;
static int client_last_frame[MAX_CLIENTS] = {-1};
static Bitrate client_bitrates[MAX_CLIENTS] = {HD};

void init_encoder(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error al abrir el archivo de video");
        exit(EXIT_FAILURE);
    }

    char buffer[4096];
    if (fgets(buffer, sizeof(buffer), file)) {
        char *token = strtok(buffer, ",");
        while (token) {
            frames = realloc(frames, sizeof(int) * (total_frames + 1));
            frames[total_frames++] = atoi(token);
            token = strtok(NULL, ",");
        }
    }
    fclose(file);
}

int get_next_frame(int client_id) {
    if (client_id < 0 || client_id >= MAX_CLIENTS || !frames) {
        return -1;
    }

    Bitrate bitrate = client_bitrates[client_id];
    int step = (bitrate == HD) ? 1 : (bitrate == MD) ? 10 : 100;

    if (client_last_frame[client_id] == -1) {
        client_last_frame[client_id] = 0;
        return frames[0];
    }

    int current_frame = client_last_frame[client_id] + step;

    if (current_frame >= total_frames) {
        return -1;
    }

    client_last_frame[client_id] = current_frame;

    return frames[current_frame];
}



void set_client_bitrate(int client_id, Bitrate bitrate) {
    if (client_id < 0 || client_id >= MAX_CLIENTS) return;
    client_bitrates[client_id] = bitrate;
}

void free_encoder() {
    if (frames) {
        free(frames);
        frames = NULL;
    }
}
