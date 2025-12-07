#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"

void print_main_menu() {
    clear_screen();
    printf("======================================\n");
    printf("      P2P FILE SHARING SYSTEM\n");
    printf("======================================\n");
    printf(" [1] Share a file\n");
    printf(" [2] Unshare a file\n");
    printf(" [3] Search for file\n");
    printf(" [4] Download file\n");
    printf(" [5] Show my shared files (Local)\n");
    printf(" [0] Logout\n");
    printf("======================================\n");
}

void print_auth_menu() {
    clear_screen();
    printf("======================================\n");
    printf("      P2P FILE SHARING SYSTEM\n");
    printf("======================================\n");
    printf(" [1] Register Account\n");
    printf(" [2] Login\n");
    printf(" [0] Exit Application\n");
    printf("======================================\n");
}
void pause_screen(){
    printf("\nPress Enter to continue...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // clean buffer
}

void clear_screen() {
    system("clear");
}

int get_choice(){
    int choice = -1;
    char buffer[50];

    printf(">> Enter your choice: ");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (sscanf(buffer, "%d", &choice) != 1) {
            choice = -1;
        }
    }
    return choice;
}

void get_input(char *msg, char* buffer, int size) {
    printf("%s", msg);               
    fgets(buffer, size, stdin);      
    
    buffer[strcspn(buffer, "\n")] = 0; // clean trailling /n 
}