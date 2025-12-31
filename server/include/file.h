#ifndef FILE__H
#define FILE__H

#include "protocol.h"

void handle_share_file(int sock, file_entry_t *req);

void handle_unshare_file(int sock, file_entry_t *req);

void handle_search_file(int sock, search_file_req_t *req);
#endif 