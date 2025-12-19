#ifndef MENU_H
#define MENU_H


// UI
void print_main_menu();
void print_auth_menu();

// Input Handler
int get_choice();
void get_input(char *msg, char *buffer, int max_len);



#endif