#ifndef PEER__H
#define PEER__H

#include "protocol.h"

void do_init_peer();
void do_register_peer(int server_sock, uint32_t client_id);

#endif