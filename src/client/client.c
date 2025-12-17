#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "protocol.h"
#include "network_utils.h"

int server_sock; 
uint32_t client_id;
char current_user[MAX_USERNAME_LEN];
int is_logged_in = 1;

void do_register();
void do_login();
void do_share_file();

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_Addr> <Port_Number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *ip_addr = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_addr;

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_addr, &server_addr.sin_addr) <= 0) {
        perror("inet_pton() error");
        exit(EXIT_FAILURE);
    }

    if (connect(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect() error");
        exit(EXIT_FAILURE);
    }

    int option;
    // Main  loop
    while (1) {
        if (!is_logged_in) {
            print_auth_menu();
            option = getOption();
            switch (option) {
                case 1: do_register(); break;
                case 2: do_login(); break;
            }
        } else {
            print_main_menu();
            option = getOption();
            switch (option) {
                case 1: do_register_p2p_port(); break;
                case 2: do_share_file(); break;
                case 3: do_unshare_file(); break;
            }
        }
    }

    close(server_sock);
    return 0;
}

void do_register(){
    
}

void do_login(){

}

void do_register_p2p_port(){
    uint16_t p2p_port;
    char buffer[10];

    printf("\n--- REGISTER P2P PORT ---\n");

    // Get port from stdin
    get_input("Enter a port to listen P2P requests: ", buffer, sizeof(buffer));
    p2p_port = (uint16_t) atoi(buffer);

    // Send request 
    register_p2p_port_req_t req; 
    memset(&req, 0, sizeof(req));

    req.client_id = htonl(client_id); 
    req.p2p_port = htons(p2p_port);
    
    if (send_message(server_sock, MSG_REGISTER_P2P_POR_REQ, &req, sizeof(req)) < 0){
        perror("send_message() error"); 
    }
}

void do_share_file(){
    char file_name[MAX_FILENAME_LEN];
    char file_path[MAX_FILEPATH_LEN];

    printf("\n--- SHARE NEW FILE ---\n");
    // 1. Get input from stdin
    get_input("Enter file name to share: ", file_name, sizeof(file_name));

    get_input("Enter an absolute path to your file: ", file_path, sizeof(file_path));

    // 2. Save in local index file
    FILE *f_index = fopen(INDEX_FILE, "a");
    if (f_index == NULL) {
        printf("Cannot open %s to save.\n", INDEX_FILE);
        return;
    }

    fprintf(f_index, "%s %s\n", file_name, file_path);
    fclose(f_index);
    
    // 3. Send request
    
    // Create request
    share_file_req_t req; 
    memset(&req, 0, sizeof(req));

    req.client_id = htonl(client_id);
    strcpy(req.file_name, file_name);

    // Send 
    if (send_message(server_sock, MSG_SHARE_FILE_REQ, &req, sizeof(req)) < 0) {
        perror("send_message() error"); 
    } 
}

