#include <stdio.h>
#include <string.h>
#include "account.h"

static Account accounts[MAX_ACCOUNTS];  // Array to store accounts
static int num_accounts = 0;            // Number of loaded accounts

void load_accounts(){
    FILE *f = fopen(ACCOUNT_FILE, "r");
    if (f == NULL) {
        f = fopen(ACCOUNT_FILE, "w"); 
        if (f) fclose(f);
        return; 
    }

    num_accounts = 0;
    char line[256];
    while (fgets(line, sizeof(line), f) != NULL && num_accounts < MAX_ACCOUNTS){
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) > 0){
            char username[MAX_USERNAME_LEN];
            char password[MAX_PASSWORD_LEN];
            if (sscanf(line, "%s %s", username, password) == 2) {
                strcpy(accounts[num_accounts].username, username);
                strcpy(accounts[num_accounts].password, password);
                accounts[num_accounts].status = 0;
                num_accounts ++;
            }
        }
    }

    printf("Loaded %d accounts from account.txt\n", num_accounts);
    fclose(f);
}

static Account* find_account_by_username(char *username) {
    if (username == NULL) return NULL;

    for (int i = 0; i < num_accounts; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            return &accounts[i]; 
        }
    }
    return NULL; 
}

void register_account(char *username, char *password){
    if (num_accounts >= MAX_ACCOUNTS) { // check if reach maximum number of accounts
        return DATABASE_FULL;
    }

    if (find_account_by_username(username) != NULL) { // check if username already taken
        return USERNAME_ALREADY_TAKEN;
    }

    // open file
    FILE *f = fopen(ACCOUNT_FILE, "a");
    if (f == NULL) {
        return SYSTEM_ERROR; 
    }

    fprintf(f, "%s %s\n", username, password);
    fclose(f);
    // Update accounts in server 
    strcpy(accounts[num_accounts].username, username);
    strcpy(accounts[num_accounts].password, password);
    accounts[num_accounts].status = 0;
    num_accounts++;

    return SUCCESS;
}

int login_account(char *username, char *password) {
    Account *acc = find_account_by_username(username);
    
    if (acc == NULL) {
        return USER_NOT_FOUND; 
    }

    if (strcmp(acc->password, password) != 0) {
        return BAD_CREDENTIALS; 
    }

    if (acc->status == 1) {
        return ALREADY_LOGGED_IN;
    }

    acc->status = 1;
    return SUCCESS; 
}

int logout_account(const char* username){
    Account *acc = find_account_by_username(username);
    if (acc == NULL) {
        return USER_NOT_FOUND; 
    }
    
    acc->status = 0;
    return SUCCESS;
}
