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
    send_message(sock, MSG_SHARE_FILE_RES, &res, sizeof(res));
}