#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "protocol.h"
#include "network_utils.h"
#include "menu.h"
#include "auth.h"
#include "peer.h"
#include "file.h"

int server_sock; 
uint32_t client_id = 305419896;
char current_user[MAX_USERNAME_LEN];
int is_logged_in = 1;


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
            option = get_choice();
            switch (option) {
                case 1: do_register(); break;
                case 2: do_login(); break;
            }
        } else {
            print_main_menu();
            option = get_choice();
            switch (option) {
                case 1: 
                    do_register_peer(server_sock, client_id); 
                    pause_screen();
                    break;
                case 2: 
                    do_share_file(server_sock, client_id); 
                    pause_screen();
                    break;
                case 3: 
                    do_unshare_file(server_sock, client_id); 
                    pause_screen();
                    break;
            }
        }
    }
    close(server_sock);
    return 0;
}

<<<<<<< HEAD:src/client/client.c
void do_register(){

}

void do_login(){
    login_req_t req;
    login_res_t res;
    uint8_t msg_type;
    void *payload = NULL;

    printf("--- LOGIN ---\n");
    printf("Username: ");
    scanf("%s", req.username);
    printf("Password: ");
    scanf("%s", req.password);

    // Send login request
    if (send_message(client_sock, MSG_LOGIN_REQ, &req, sizeof(req)) < 0) {
        printf("Error sending login request\n");
        return;
    }

    // Receive login response
    if (recv_message(client_sock, &msg_type, &payload) < 0) {
        printf("Error receiving login response\n");
        return;
    }

    if (msg_type == MSG_LOGIN_RES && payload != NULL) {
        memcpy(&res, payload, sizeof(res));
        
        switch (res.status) {
            case STATUS_SUCCESS:
                printf("Login successful!\n");
                strcpy(current_user, req.username);
                is_logged_in = 1;
                break;
            case STATUS_ERR_USERNAME_NOTFOUND:
                printf("Error: Username not found\n");
                break;
            case STATUS_ERR_BAD_CREDENTIALS:
                printf("Error: Incorrect password\n");
                break;
            case STATUS_ERR_ALREADY_LOGGED_IN:
                printf("Error: User already logged in\n");
                break;
            case STATUS_ERR_FAILURE:
                printf("Error: Login failed\n");
                break;
            default:
                printf("Error: Unknown status code %d\n", res.status);
                break;
        }
    }

    if (payload) {
        free(payload);
    }
}

=======
>>>>>>> be0d1a8603d2c43425fe3ca19778fc942fde061a:client/src/main.c
