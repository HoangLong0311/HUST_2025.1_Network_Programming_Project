#ifndef P2P__H
#define P2P__H

#include "protocol.h"

void start_p2p_listener(uint16_t p2p_port);
void download_file(const char *ip, int port, const char *file_name);

#endif