#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include "network_utils.h"
#include "p2p.h"

static void *handle_peer(void *arg) {
    int peer_sock = (int)(intptr_t)arg;
    uint8_t msg_type;
    void *payload = NULL; 
    int len; 
    
    while ((len = recv_message(peer_sock, &msg_type, &payload)) > 0){
        if (msg_type == MSG_P2P_DOWNLOAD_REQ) {
            download_file_req_t *req = (download_file_req_t *)payload;
            
            // check in index file
            FILE *f_index = fopen(INDEX_FILE, "r");
            if (f_index == NULL) {
                printf("Cannot open %s to save.\n", INDEX_FILE);
                close(peer_sock);
                return NULL;
            } 

            int ret = 0;
            char line[MAX_FILENAME_LEN + MAX_FILEPATH_LEN + 10];
            int status; 
            char cur_file_name[MAX_FILENAME_LEN], cur_file_path[MAX_FILEPATH_LEN];
            char founded_path[MAX_FILEPATH_LEN];
            while(fgets(line, sizeof(line), f_index) != NULL) {
                if (sscanf(line, "%d %s %s", &status, cur_file_name, cur_file_path) == 3) {
                    if (strcmp(cur_file_name, req->file_name) == 0 && status == 1) { 
                        ret = 1;
                        strcpy(founded_path, cur_file_path); 
                        break;
                    }
                }
            }
            fclose(f_index);

            // if not found
            if (ret == 0) {
                send_message(peer_sock, MSG_P2P_DOWNLOAD_ERR, NULL, 0);
            } else { // founded
                // check in physical disk 
                struct stat st;
                if (stat(founded_path, &st) == -1) {
                    send_message(peer_sock, MSG_P2P_DOWNLOAD_ERR, NULL, 0);
                    
                } else {
                    long file_size = st.st_size;
                    download_file_res_t res;
                    memset(&res, 0, sizeof(download_file_res_t));
                    res.file_size = htonl((uint32_t)file_size);

                    // send file metadata first
                    if (send_message(peer_sock, MSG_P2P_FILE_METADATA, &res, sizeof(res)) < 0) {
                        perror("send_message() error");
                        break;
                    }

                    // send file binaries
                    if (send_file_data(peer_sock, founded_path) < 0) {
                        perror("send_file_data() error");
                    } 
                }
            }
        }
        // clean
        free(payload);
        if (payload) payload = NULL; 
        break;
    }

    close(peer_sock);
    return NULL;
}

static void *listener_loop(void *arg) {
    int listen_sock = (int)(intptr_t)arg;
    int conn_sock;
    struct sockaddr_in peer_addr;
    socklen_t addr_len = sizeof(peer_addr);

    while (1) {
        // wait for connection
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&peer_addr, &addr_len)) == -1) {
            perror("accept() error");
            continue;
        }

        // create a new thread to handle request
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_peer, (void *)(intptr_t)conn_sock) != 0) {
            perror("pthread_create() error");
            close(conn_sock);
        } else {
            // detach thread
            pthread_detach(tid);
        }
    }
    return NULL;
}

void start_p2p_listener(uint16_t p2p_port) {
    int listen_sock;
    struct sockaddr_in server_addr;

    // Create a socket
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    // fix already in use when bind
    int opt = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt() error");
        exit(EXIT_FAILURE);
    }

    // Bind socket 
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(p2p_port);

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind() error");
        exit(EXIT_FAILURE);
    }

    // Listening
    if (listen(listen_sock, BACKLOG) < 0) {
        perror("listen() error");
        exit(EXIT_FAILURE);
    }

    // Run the listener loop in a thread

    pthread_t tid;
    if (pthread_create(&tid, NULL, listener_loop, (void *)(intptr_t)listen_sock) != 0) {
        perror("pthread_create() error");
        exit(EXIT_FAILURE);
    }
    
    // detach the thread
    pthread_detach(tid);
}

void download_file(const char *ip, int port, const char *file_name) {
    int peer_sock;
    struct sockaddr_in peer_addr;

    if ((peer_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() error");
        return; 
    }

    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(port); 

    if (inet_pton(AF_INET, ip, &peer_addr.sin_addr) <= 0) {
        perror("inet_pton() error");
        close(peer_sock);
        return;
    }

    if (connect(peer_sock, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) == -1) {
        perror("connect() error");
        close(peer_sock);
        return;
    }

    printf(" Connected to Peer %s:%u. Sending request for file: %s\n", ip, port, file_name);
    
    // create request
    download_file_req_t req;
    memset(&req, 0, sizeof(download_file_req_t));

    strcpy(req.file_name, file_name);

    // send message 
    if (send_message(peer_sock, MSG_P2P_DOWNLOAD_REQ, &req, sizeof(download_file_req_t)) < 0) {
        perror("send_message() error");
        close(peer_sock);
        return;
    }

    // receive message and data
    uint8_t msg_type;
    void *payload = NULL;
    int len; 

    // receive message to check downloadable 
    while((len = recv_message(peer_sock, &msg_type, &payload)) > 0) {
        if (msg_type == MSG_P2P_DOWNLOAD_ERR) {
            printf("Failed to download file from this peer.\n");
        } else if (msg_type == MSG_P2P_FILE_METADATA) {
            download_file_res_t *res = (download_file_res_t *)payload;
            long file_size = (long) ntohl(res->file_size);

            printf("File found. Size: %ld bytes. Receiving data stream...\n", file_size);

            // clean
            free(payload);
            if (payload) payload = NULL;

            // receive data stream
            if (recv_file_data(peer_sock, file_size, (char*)file_name) == 0) {
                printf("!!! Download successfully: %s\n", file_name);
            } else {
                printf("!!! Failed to download file from this peer.\n");
            }

            break;
        }
    }
    
    close(peer_sock);
}