#include "file.h"
#include "index.h"
#include "network_utils.h"

void handle_share_file(int sock, file_entry_t *req){
    share_file_res_t res;
    memset(&res, 0, sizeof(res));
    uint32_t client_id = ntohl(req->client_id);
    // add file to index db
    int ret = add_file(client_id, req->file_name);
    switch (ret) {
    case SUCCESS:
        res.status = STATUS_SUCCESS;
        break;
    case PEER_NOT_FOUND:
        res.status = STATUS_ERR_PEER_NOT_FOUND;
        break; 
    case FILE_ALREADY_SHARED:
        res.status = STATUS_ERR_FILE_ALREADY_SHARED;
        break;
    default:
        res.status = STATUS_FAILURE;
        break;
    }

    // send message
    if (send_message(sock, MSG_SHARE_FILE_RES, &res, sizeof(res)) < 0){
        perror("send_message() error");
        return;
    }
}

void handle_unshare_file(int sock, file_entry_t *req){
    unshare_file_res_t res;
    memset(&res, 0, sizeof(unshare_file_res_t));
    uint32_t client_id = ntohl(req->client_id);

    int ret = remove_file(client_id, req->file_name);
    switch (ret) {
    case SUCCESS:
        res.status = STATUS_SUCCESS;
        break;
    case PEER_NOT_FOUND: 
        res.status = STATUS_ERR_PEER_NOT_FOUND;
        break;
    case FILE_NOT_FOUND:
        res.status = STATUS_ERR_FILE_NOT_FOUND;
        break;
    default:
        res.status = STATUS_FAILURE;
        break;
    }

    if (send_message(sock, MSG_UNSHARE_FILE_RES, &res, sizeof(res)) < 0){
        perror("send_message() error");
        return;
    }
}

void handle_search_file(int sock, search_file_req_t *req){
    search_file_metadata_t metadata;
    memset(&metadata, 0, sizeof(search_file_metadata_t));
    SearchResult result; 
    memset(&result, 0, sizeof(SearchResult));

    search_file(&result, req->file_name);

    // extract data and wrap in message
    metadata.contact_count = htons(result.contact_count); 
    size_t payload_len = sizeof(search_file_metadata_t) + (result.contact_count * sizeof(contact_t));
    uint8_t *payload = (uint8_t *) malloc(payload_len);
    // copy metadata into buffer
    memcpy(payload, &metadata, sizeof(metadata));

    // pointer to contacts part in buffer
    contact_t *contacts = (contact_t *)(payload + sizeof(search_file_metadata_t));
    // step variable
    int i = 0;
    if (metadata.contact_count > 0){
        PeerContact *current_node = result.contacts_head;
        while(current_node != NULL){
            // create contact from node data
            strcpy(contacts[i].client_ip, current_node->client_ip);
            contacts[i].p2p_port = htons(current_node->p2p_port);
            // update
            current_node = current_node->next;
            i++;
        }
    }
    // send message
    if (send_message(sock, MSG_SEARCH_FILE_RES, payload, payload_len) < 0) {
        perror("send_message() error");
        free(payload);
        if (payload) payload = NULL;
        free_search_result(&result);
        return;
    }

    // clean 
    free(payload);
    if (payload) payload = NULL;
    free_search_result(&result);
}