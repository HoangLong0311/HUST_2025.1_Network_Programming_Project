#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <stddef.h>

int send_all(int sockfd, void *data, size_t len);
int recv_all(int sockfd, void *data, size_t len);

int send_message(int sockfd, uint8_t msg_type, void *payload, uint16_t payload_len);
int recv_message(int sockfd, uint8_t *msg_type, void **payload);

#endif