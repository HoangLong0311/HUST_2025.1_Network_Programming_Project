#include "peer.h"
#include "index.h"
#include "network_utils.h"

void handle_register_peer(int sock, char* client_ip, register_peer_req_t *req){
    register_peer_res_t res;
    memset(&res, 0, sizeof(res));

    int ret = register_peer(req->client_id, client_ip, req->p2p_port);
    switch(ret) {
        case SUCCESS: 
            res.status = STATUS_SUCCESS;
            break;
        case CLIENT_ID_ALREADY_EXISTS: 
            res.status = STATUS_ERR_DUPLICATED_PEER;
            break;
        default: 
            res.status = STATUS_FAILURE;
            break;
    }

    send_message(sock, MSG_REGISTER_PEER_RES, &res, sizeof(res));
}
