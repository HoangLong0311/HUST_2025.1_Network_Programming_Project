#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "index.h"

static PeerNode *head = NULL;

static PeerNode* find_peer_by_id(uint32_t client_id){
    PeerNode *cur = head;
    while (cur != NULL){
        if (cur->client_id == client_id){
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

int init_peer(uint32_t client_id, char* client_ip, uint16_t p2p_port){
    // find duplicated
    PeerNode *peer;
    if ((peer = find_peer_by_id(client_id)) != NULL) {
        return CLIENT_ID_ALREADY_EXISTS;
    }
    // create new peer
    PeerNode *newPeer = (PeerNode *) malloc(sizeof(PeerNode));
    memset(&newPeer, 0, sizeof(newPeer));
    strcpy(newPeer->client_ip, client_ip);
    newPeer->p2p_port = p2p_port;
    // update head 
    newPeer->next = head; 
    head = newPeer;
    return SUCCESS;
}

int add_file(uint32_t client_id, char *file_name){
    // find peer
    PeerNode *peer;
    if ((peer = find_peer_by_id(client_id)) == NULL) {
        return PEER_NOT_FOUND;
    }
    
    // check duplicated
    for (int i = 0; i < peer->file_count; ++i) {
        if (strcmp(peer->files[i].file_name, file_name) == 0) {
            return FILE_ALREADY_SHARED;
        }
    }

    // append to files
    int idx = peer->file_count;
    strcpy(peer->files[idx].file_name, file_name);
    peer->file_count++;
    return SUCCESS;
}


int register_peer(uint32_t client_id, char* client_ip, uint16_t p2p_port){
    // find duplicated
        PeerNode *peer;
    if ((peer = find_peer_by_id(client_id)) == NULL) {
        return PEER_NOT_FOUND;
    } 
    peer->p2p_port = p2p_port;
    strcpy(peer->client_ip, client_ip);
    return SUCCESS;
}

