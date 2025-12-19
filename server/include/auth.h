#ifndef AUTH__H
#define AUTH__H

#include "protocol.h"
void handle_register(int sock, register_req_t *req);
void handle_login(int sock, login_req_t *req);
void handle_logout(int sock, logout_req_t *req);

#endif