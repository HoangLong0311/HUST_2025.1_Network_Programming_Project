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

int init_peer(uint32_t client_id, char *client_ip, uint16_t p2p_port){
    // find duplicated
    PeerNode *peer;
    if ((peer = find_peer_by_id(client_id)) != NULL) {
        return CLIENT_ID_ALREADY_EXISTS;
    }
    // create new peer
    PeerNode *newPeer = (PeerNode *) malloc(sizeof(PeerNode));
    memset(newPeer, 0, sizeof(PeerNode));
    newPeer->client_id = client_id;
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

int register_peer(uint32_t client_id, char *client_ip, uint16_t p2p_port){
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
    init_peer(1001, "127.0.0.1", 20001);
    init_peer(1002, "127.0.0.1", 20002);
    init_peer(1003, "127.0.0.1", 20003);

    printf("Init sample data... 3 Peers created.\n");
}
int remove_file(uint32_t client_id, char *file_name){
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

void search_file(SearchResult *result, char *file_name){
    PeerNode *current = head;
    while(current != NULL) {
        for(int i = 0; i < current->file_count; ++i) {
            if (strcmp(current->files[i].file_name, file_name) == 0){
                PeerContact *new_contact = (PeerContact *) malloc(sizeof(PeerContact)); 
                if (new_contact == NULL) break;

                strcpy(new_contact->client_ip, current->client_ip);
                new_contact->p2p_port = current->p2p_port;

                new_contact->next = result->contacts_head;
                result->contacts_head = new_contact;
                result->contact_count ++;
                break;
            }
        }
        current = current->next;
    }
}

void free_search_result(SearchResult *result){
    if (result == NULL) return;

    PeerContact *current = result->contacts_head;
    PeerContact *temp;
    
    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);
    }
}