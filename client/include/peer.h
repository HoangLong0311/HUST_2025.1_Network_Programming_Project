#ifndef PEER__H
#define PEER__H

#include "protocol.h"

void load_client_id(uint32_t *client_id);
uint32_t generate_client_id();
void do_init_peer(int server_sock, uint32_t client_id, uint16_t p2p_port);
void do_register_peer(int server_sock, uint32_t client_id);
void start_p2p_listener(uint16_t p2p_port);

#endif