#ifndef FILE__H
#define FILE__H

#include "protocol.h"

void handle_share_file(int sock, file_entry_t *req);

void handle_unshare_file(int sock, file_entry_t *req);
#endif 