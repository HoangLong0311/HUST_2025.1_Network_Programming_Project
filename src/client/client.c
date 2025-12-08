#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "protocol.h"
#include "network_utils.h"

int client_sock; 
char current_user[MAX_USERNAME_LEN];
int is_logged_in = 0;

void do_register();
void do_login();

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_Addr> <Port_Number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *ip_addr = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_addr;

    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_addr, &server_addr.sin_addr) <= 0) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }

    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect()");
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
        }
    }

    close(client_sock);
    return 0;
}

void do_register(){
    
}

void do_login(){

}