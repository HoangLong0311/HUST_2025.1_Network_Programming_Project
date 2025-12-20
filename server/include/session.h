#ifndef SESSION_H
#define SESSION_H

#include <pthread.h>
#include "protocol.h"

typedef struct Session{
    int conn_sock;                      // connect socket
    char username[MAX_USERNAME_LEN];    // username in session
} Session;

void init_sessions();
int add_session(int sock, char* username);
void remove_session(int sock);
int is_user_online(char* username);
int check_session_owner(int sock, char* username);

#endif