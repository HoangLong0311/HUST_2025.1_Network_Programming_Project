#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include "account.h"   
#include "protocol.h"
#include "network_utils.h"
#include "index.h"

void handle_client(void *arg);

void handle_register(int sock, register_req_t *req);
void handle_login(int sock, login_req_t *req);
void handle_logout(int sock, logout_req_t *req);
void handle_register_peer(int sock, char* client_ip, peer_info_t *req);
void handle_share_file(int sock, share_file_req_t *req);

int main(int argc, char *argv[]){
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    pthread_t tid;

    if (argc != 2) {
        printf("Usage: %s Port_Number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    // 1. Load accounts when initialize server
    load_accounts();
    // 2. Create socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); 
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // 3. Bind 
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("bind() error");
        exit(EXIT_FAILURE);
    } 

    // 4. Listen
    if (listen(server_sock, BACKLOG) == 0){
        printf("Server is running on port %d...\n", port);
    } else {
        perror("listen() error");
        exit(EXIT_FAILURE);
    }

    sin_size = sizeof(struct sockaddr_in);
    // Main loop
    while(1) {
        if ((client_sock = accept(server_sock, (struct sockaddr*)&client_addr, (socklen_t*) &sin_size)) == -1) {
            perror("accept() error");
        } 
        client_info_t *client_info = malloc(sizeof(client_info_t));
        client_info->sock = client_sock; 
        if (inet_ntop(AF_INET, &client_addr.sin_addr, client_info->ip, MAX_IP_LEN) == NULL){
            perror("inet_ntop() error:");
        } 
        int client_port = ntohs(client_addr.sin_port);
        printf("New connection from %s:%d\n", client_info->ip, client_port);
        
        pthread_create(&tid, NULL, &handle_client, (void*)client_info);
        free(client_info);
    }
    clost(server_sock);
}

void handle_client(void *arg) {
    client_info_t *client_info = (client_info_t*) arg;
    int sock = client_info->sock;
    char client_ip[MAX_IP_LEN];
    strcpy(client_ip, client_info->ip);
    free(arg);
    free(client_info);
    
    uint8_t msg_type;
    void *payload = NULL; 
    int len;
    
    while ((len = receive_message(sock, &msg_type, &payload)) > 0) {
        switch(msg_type) {
            case MSG_REGISTER_REQ: 
                handle_register(sock, (register_req_t *) payload);
                break;
            case MSG_LOGIN_REQ: 
                handle_login(sock, (login_req_t *) payload);
                break;
            case MSG_LOGOUT_REQ:
                handle_logout(sock, (logout_req_t *) payload);
            case MSG_REGISTER_PEER_REQ: 
                handle_register_p2p_port(sock, client_ip, (peer_info_t*) payload);
            case MSG_SHARE_FILE_REQ: 
                handle_share_file(sock, (share_file_req_t *)payload);
        }
        // Clean payload
        if (payload) {
            free(payload);
            payload = NULL;
        }
    }

    printf("Socket %d disconnected.\n", sock);
    remove_session(sock);
    thread_detach(pthread_self());
    close(sock);
    pthread_exit(NULL);
}

void handle_register(int sock, register_req_t *req) {
    register_res_t res;
    
    int ret = register_account(req->username, req->password);
    switch (ret) {
        case SUCCESS: res.status = STATUS_SUCCESS;
        case USERNAME_ALREADY_TAKEN: res.status = STATUS_ERR_USERNAME_ALREADY_TAKEN;
        default: res.status = STATUS_FAILURE;
    }
    // Send reply to client
    send_message(sock, MSG_REGISTER_RES, &res, sizeof(res));
}

void handle_login(int sock, login_req_t *req) {
    login_res_t res; 
    
    int result = login_account(req->username, req->password);
    
    switch (result) {
        case SUCCESS:
            if (is_user_online(req->username)) {
                res.status = STATUS_ERR_ALREADY_LOGGED_IN;
            } else {
                int session_idx = add_session(sock, req->username);

                if (session_idx == -1){
                    res.status = STATUS_FAILURE;
                } else {
                    res.status = STATUS_SUCCESS;
                }
            }
        case USER_NOT_FOUND:
            res.status = STATUS_ERR_USERNAME_NOTFOUND;
        case BAD_CREDENTIALS:
            res.status = STATUS_ERR_BAD_CREDENTIALS;
        default:
            res.status = STATUS_FAILURE;
    }

    // Send reply to client
    send_message(sock, MSG_LOGIN_RES, &res, sizeof(res));
}

void handle_logout(int sock, logout_req_t *req) {
    logout_res_t res;
    int ret;

    // Check owner first
    if (!check_session_owner(sock, req->username)) {
        res.status = STATUS_ERR_UNAUTHORIZED; 
        send_message(sock, MSG_LOGOUT_RES, &res, sizeof(res));
        return;
    }
    // Call logout 
    if ((ret = logout_account(req->username)) == SUCCESS) {
        remove_session(sock);
        res.status = STATUS_SUCCESS;
    } 
    else {
        res.status = STATUS_ERR_USERNAME_NOTFOUND;
    }
    // send reply to client
    send_message(sock, MSG_LOGOUT_RES, &res, sizeof(res));
}

void handle_register_peer(int sock, char* client_ip, peer_info_t *req){
    register_peer_res_t res;
    memset(&res, 0, sizeof(res));

    int ret = register_peer(req->client_id, client_ip, req->p2p_port);
    switch(ret) {
        case SUCCESS: 
            res.status = STATUS_SUCCESS;
            break;
        case CLIENT_ID_ALREADY_EXISTS: 
            res.status = STATUS_ERR_DUPLICATED_PEER;
        default: 
            res.status = STATUS_FAILURE;
            break;
    }

    send_message(sock, MSG_REGISTER_PEER_RES, &res, sizeof(res));
}

void handle_share_file(int sock, share_file_req_t *req){
    share_file_res_t res;
    memset(&res, 0, sizeof(res));
    // add file to index db
    int ret = add_file(req->client_id, req->file_name);
    switch (ret) {
    case SUCCESS:
        res.status = STATUS_SUCCESS;
        break;
    case PEER_NOT_FOUND:
        res.status = STATUS_FAILURE;
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