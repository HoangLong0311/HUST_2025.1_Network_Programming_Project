#ifndef SESSION_H
#define SESSION_H

#include <pthread.h>
#include "protocol.h"


typedef struct {
    int conn_sock;                      // connect socket
    char username[MAX_USERNAME_LEN];    // username in session
} Session;

Session sessions[MAX_CLIENTS]; // Online sessions

// create mutex for session
pthread_mutex_t sessions_mutex = PTHREAD_MUTEX_INITIALIZER;

#endif