#ifndef FILE__H
#define FILE__H

#include "protocol.h"

void do_share_file(int server_sock, uint32_t client_id);

void do_unshare_file(int server_sock, uint32_t client_id);

#endif