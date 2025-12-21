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
                case 1: do_register_peer(server_sock, client_id); break;
                case 2: do_share_file(server_sock, client_id); break;
                case 3: do_unshare_file(server_sock, client_id); break;
            }
        }
    }
    close(server_sock);
    return 0;
}

