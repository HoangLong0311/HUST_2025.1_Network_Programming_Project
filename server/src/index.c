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
            printf("[DEBUG] already_shared_file_name: %s", peer->files[i].file_name);
            printf("[DEBUG] file_name: %s", file_name);
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

void init_sample_data() {
    PeerNode *test_node = malloc(sizeof(PeerNode));
    memset(test_node, 0, sizeof(PeerNode));

    test_node->client_id = 305419896;
    strcpy(test_node->client_ip, "127.0.0.1");
    test_node->p2p_port = 50000;
    test_node->file_count = 0; 
    test_node->next = NULL;
    head = test_node; 

    printf("Test node initialized: Client ID %u is listening on %s:%d\n", 
            head->client_id, head->client_ip, head->p2p_port);
}

int remove_file(uint32_t client_id, char* file_name){
    // find peer
    PeerNode *peer;
    if ((peer = find_peer_by_id(client_id)) == NULL) {
        return PEER_NOT_FOUND;
    }
    
    for (int i = 0; i < peer->file_count; ++i) {
        if (strcmp(peer->files[i].file_name, file_name) == 0) {
            if (i != peer->file_count - 1) {
                peer->files[i] = peer->files[peer->file_count - 1];
            }
            peer->file_count--;
            return SUCCESS;
        }
    }
    return FILE_NOT_FOUND;
}