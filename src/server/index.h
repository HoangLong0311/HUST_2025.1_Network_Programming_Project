#ifndef INDEX__H
#define INDEX__H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <protocol.h>

#define SUCCESS 1
#define PEER_NOT_FOUND -1
#define FILE_ALREADY_SHARED -2
#define CLIENT_ID_ALREADY_EXISTS -3

typedef struct {
    char file_name[MAX_FILENAME_LEN];
} FileEntry;

typedef struct {
    uint32_t client_id;
    char client_ip[MAX_IP_LEN];
    int p2p_port;
    FileEntry files[MAX_FILES_PER_PEER];
    int file_count;
    struct PeerNode *next;
} PeerNode;

int add_file(uint32_t client_id, char *file_name);

int register_peer(uint32_t client_id, char* client_ip, uint16_t p2p_port);

#endif