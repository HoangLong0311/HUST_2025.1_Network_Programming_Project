#ifndef PEER__H
#define PEER__H

#include "protocol.h"

void handle_register_peer(int sock, char* client_ip, peer_info_t* req);

#endif