#ifndef PEER__H
#define PEER__H

#include "protocol.h"

void handle_register_peer(int sock, char* client_ip, register_peer_req_t *req);

#endif