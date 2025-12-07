#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <protocol.h>

#define SUCCESS 1 
#define USER_NOT_FOUND 2
#define BAD_CREDENTIALS 3
#define ALREADY_LOGGED_IN 4

#define USERNAME_ALREADY_TAKEN 5
#define DATABASE_FULL 6
#define SYSTEM_ERROR -1

typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int status; // 1 - online, 0 - offline
} Account;

void load_accounts();
void register_account(char *username, char *password);
int login_account(char* username, char* password);
void logout_account(char* username);

#endif

