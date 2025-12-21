#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include "file.h"
#include "network_utils.h"
#include "menu.h"


void do_share_file(int server_sock, uint32_t client_id){
    char file_name[MAX_FILENAME_LEN];
    char file_path[MAX_FILEPATH_LEN];

    printf("\n--- SHARE NEW FILE ---\n");
    // Get input from stdin

    get_input("Enter an absolute path to your file: ", file_path, sizeof(file_path));
    strcpy(file_name, basename(file_path));
    
    // Create request
    file_entry_t req; 
    memset(&req, 0, sizeof(req));

    req.client_id = htonl(client_id);
    strcpy(req.file_name, file_name);

    // Send 
    if (send_message(server_sock, MSG_SHARE_FILE_REQ, &req, sizeof(req)) < 0) {
        perror("send_message() error"); 
    } 

    // Receive message
    void *payload = NULL;
    uint8_t msg_type; 
    int len; 
    while((len = recv_message(server_sock, &msg_type, &payload)) > 0){
        if (msg_type == MSG_SHARE_FILE_RES) {
            share_file_res_t *res = (share_file_res_t*) payload;
            switch (res->status) {
            case STATUS_SUCCESS:
                printf("File %s is now visible to peers.\n", file_name);
                // Save in local index file
                FILE *f_index = fopen(INDEX_FILE, "a");
                if (f_index == NULL) {
                    printf("Cannot open %s to save.\n", INDEX_FILE);
                    return;
                }

                fprintf(f_index, "%d %s %s\n", 1, file_name, file_path);
                fclose(f_index);
                break;
            case STATUS_ERR_PEER_NOT_FOUND:
                printf("Your client ID %u was not found. Please check config.txt or restart the application.\n", client_id);
                break;
            case STATUS_ERR_FILE_ALREADY_SHARED:
                printf("This file has already been registered for sharing.\n");
                break;
            default:
                printf("Operation failed.\n");
                break;
            }   
        }
    }
    free(payload);
    if (payload) payload = NULL;
}

void do_unshare_file(int server_sock, uint32_t client_id){
    char file_name[MAX_FILENAME_LEN];

    printf("\n--- UNSHARE A FILE ---\n");
    get_input("Enter the filename to unshare: ", file_name, sizeof(file_name));

    // Create request
    file_entry_t req; 
    memset(&req, 0, sizeof(req));

    req.client_id = htonl(client_id);
    strcpy(req.file_name, file_name);

    // Send 
    if (send_message(server_sock, MSG_UNSHARE_FILE_REQ, &req, sizeof(req)) < 0) {
        perror("send_message() error"); 
    }

    // Receive response
    void *payload = NULL;
    uint8_t msg_type; 
    int len; 
    while((len = recv_message(server_sock, &msg_type, &payload)) > 0){
        if (msg_type == MSG_UNSHARE_FILE_RES) {
            unshare_file_res_t *res = (unshare_file_res_t*) payload;
            switch (res->status) {
            case STATUS_SUCCESS:
                // Soft delete in local index file
                FILE *f_index = fopen(INDEX_FILE, "r+");
                if (f_index == NULL) {
                    printf("Cannot open %s.\n", INDEX_FILE);
                    return;
                }
                
                char line[MAX_FILENAME_LEN + MAX_FILEPATH_LEN + 10];
                int line_start_pos; 
                int status;
                char cur_file_name[MAX_FILENAME_LEN], cur_file_path[MAX_FILEPATH_LEN];
                while (1){
                    line_start_pos = ftell(f_index);
                    if (fgets(line, sizeof(line), f_index) == NULL) break;
                    if (sscanf(line, "%d %s %s", &status, cur_file_name, cur_file_path) == 3){
                        if (strcmp(cur_file_name, file_name) == 0) {
                            fseek(f_index, line_start_pos, SEEK_SET);
                            fprintf(f_index, "%d", 0);
                            break;
                        }
                    }
                }
                fclose(f_index);

                printf("File %s is now invisible to peers.\n", file_name);
                break;
            case STATUS_ERR_PEER_NOT_FOUND:
                printf("Your client ID %u was not found. Please check config.txt or restart the application.\n", client_id);
                break;
            case STATUS_ERR_FILE_NOT_FOUND:
                printf("The file %s was not found. Please check index.txt and try again later.\n", file_name);
                break;
            default:
                printf("Operation failed.\n");
                break;
            }   
        }
    }
    free(payload);
    if (payload) payload = NULL;

}