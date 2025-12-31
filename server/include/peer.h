#ifndef PEER__H
#define PEER__H

#include "protocol.h"

void handle_init_peer(int sock, char *client_ip, peer_info_t *req);
void handle_register_peer(int sock, char* client_ip, peer_info_t* req);

#endif