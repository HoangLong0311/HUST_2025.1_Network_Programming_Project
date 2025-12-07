#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include "account.h"   
#include "protocol.h"

void handle_client(void *arg);

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
        } else {
            char client_ip[MAX_IP_LEN];
            if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, MAX_IP_LEN) == NULL){
                perror("inet_ntop() error:");
            } else {
                int client_port = ntohs(client_addr.sin_port);
                printf("New connection from %s:%d\n", client_ip, client_port);
            }
            pthread_create(&tid, NULL, &handle_client, (void*)(intptr_t)client_sock);
        }
    }
    clost(server_sock);
}

void handle_client(void *arg) {
    char buffer[BUFFER_SIZE];
    int sock = (int)(intptr_t) arg;

    thread_detach(pthread_self());

    
    close(sock);
    pthread_exit(NULL);
}



