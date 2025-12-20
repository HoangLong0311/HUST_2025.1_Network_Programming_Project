#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file.h"
#include "network_utils.h"
#include "menu.h"


void do_share_file(int server_sock, uint32_t client_id){
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
