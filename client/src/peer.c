#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include "network_utils.h"
#include "menu.h"
#include "peer.h"

void do_init_peer(){
    
}

void do_register_peer(int server_sock, uint32_t client_id){
    uint16_t p2p_port;
    char buffer[10];

    printf("\n--- REGISTER PEER ---\n");

    // Get port from stdin
    get_input("Enter a port to listen P2P requests: ", buffer, sizeof(buffer));
    p2p_port = (uint16_t) atoi(buffer);

    // Send request 
    peer_info_t req; 
    memset(&req, 0, sizeof(req));

    req.client_id = htonl(client_id); 
    req.p2p_port = htons(p2p_port);
    
    if (send_message(server_sock, MSG_REGISTER_PEER_REQ, &req, sizeof(req)) < 0){
        perror("send_message() error"); 
    }
}