#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "session.h"

static Session sessions[MAX_CLIENTS];

// mutex for sessions 
static pthread_mutex_t sessions_mutex = PTHREAD_MUTEX_INITIALIZER; 

void init_sessions(){
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sessions[i].conn_sock = -1;
        sessions[i].username[0] = '\0';
    }
}

int add_session(int sock, char* username){
    pthread_mutex_lock(&sessions_mutex);
    int idx = -1; 
    for (int i = 0; i < MAX_CLIENTS; ++i){
        if (sessions[i].conn_sock == -1){
            sessions[i].conn_sock = sock;
            strcpy(sessions[i].username, username);
            idx = i;
            break;
        }
    }
    pthread_mutex_unlock(&sessions_mutex);
    return idx;
}

void remove_session(int sock) {
    pthread_mutex_lock(&sessions_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (sessions[i].conn_sock == sock) {
            sessions[i].conn_sock = -1;
            sessions[i].username[0] = '\0';
            break; 
        }
    }
    pthread_mutex_unlock(&sessions_mutex);
}

int is_user_online(char* username) {
    pthread_mutex_lock(&sessions_mutex);
    int found = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (sessions[i].conn_sock != -1 && strcmp(sessions[i].username, username) == 0) {
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&sessions_mutex);
    return found;
}

int check_session_owner(int sock, char* username){
    pthread_mutex_lock(&sessions_mutex);
    
    int is_owner = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (sessions[i].conn_sock == sock && strcmp(sessions[i].username, username) == 0) {
            is_owner = 1;
            break;
        }
    }
    
    pthread_mutex_unlock(&sessions_mutex);
    return is_owner;
}
