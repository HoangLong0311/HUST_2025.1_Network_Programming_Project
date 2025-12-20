#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include "protocol.h"
#include "network_utils.h"
#include "session.h"
#include "account.h"
#include "peer.h"
#include "file.h"
#include "auth.h"


void *handle_client(void *arg);

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
        
        pthread_create(&tid, NULL, handle_client, (void*)client_info);
    }
    close(server_sock);
}

void *handle_client(void *arg) {
    client_info_t *client_info = (client_info_t*) arg;
    int sock = client_info->sock;
    char client_ip[MAX_IP_LEN];
    strcpy(client_ip, client_info->ip);
    free(client_info);
    
    uint8_t msg_type;
    void *payload = NULL; 
    int len;
    
    while ((len = recv_message(sock, &msg_type, &payload)) > 0) {
        switch(msg_type) {
            case MSG_REGISTER_REQ: 
                handle_register(sock, (register_req_t *) payload);
                break;
            case MSG_LOGIN_REQ: 
                handle_login(sock, (login_req_t *) payload);
                break;
            case MSG_LOGOUT_REQ:
                handle_logout(sock, (logout_req_t *) payload);
                break;
            case MSG_REGISTER_PEER_REQ: 
                handle_register_peer(sock, client_ip, (register_peer_req_t*) payload);
                break;
            case MSG_SHARE_FILE_REQ: 
                handle_share_file(sock, (share_file_req_t *)payload);
                break;
        }
        // Clean payload
        if (payload) {
            free(payload);
            payload = NULL;
        }
    }

    printf("Socket %d disconnected.\n", sock);
    remove_session(sock);
    pthread_detach(pthread_self());
    close(sock);
    pthread_exit(NULL);
}

