#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "protocol.h"

#define SUCCESS 1 
#define SYSTEM_ERROR 0
#define USER_NOT_FOUND -1
#define BAD_CREDENTIALS -2

#define USERNAME_ALREADY_TAKEN -3
#define DATABASE_FULL -4


typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} Account;

void load_accounts();
int register_account(char *username, char *password);
int login_account(char* username, char* password);
int logout_account(char* username);

#endif

