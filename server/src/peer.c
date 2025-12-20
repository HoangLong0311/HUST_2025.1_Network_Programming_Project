#include "peer.h"
#include "index.h"
#include "network_utils.h"

void handle_register_peer(int sock, char *client_ip, peer_info_t *req){
    register_peer_res_t res;
    memset(&res, 0, sizeof(res));
    uint32_t client_id = ntohl(req->client_id);
    uint16_t p2p_port = ntohs(req->p2p_port);
    int ret = register_peer(client_id, client_ip, p2p_port);
    switch(ret) {
        case SUCCESS: 
            res.status = STATUS_SUCCESS;
            break;
        case PEER_NOT_FOUND: 
            res.status = STATUS_ERR_PEER_NOT_FOUND;
            break;
        default: 
            res.status = STATUS_FAILURE;
            break;
    }

    send_message(sock, MSG_REGISTER_PEER_RES, &res, sizeof(res));
}
