#ifndef ENCODER_H
#define ENCODER_H

typedef enum { HD, MD, LD } Bitrate;

void init_encoder(const char *filename);

int get_next_frame(int client_id);

void set_client_bitrate(int client_id, Bitrate bitrate);

void free_encoder();

#endif
