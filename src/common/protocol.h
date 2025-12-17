#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <arpa/inet.h>

// --- CONSTANTS ---
#define MAX_CLIENTS 200
#define MAX_ACCOUNTS 100
#define MAX_FILENAME_LEN 256
#define MAX_FILEPATH_LEN 256
#define MAX_FILES_PER_PEER 15
#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 32
#define MAX_IP_LEN INET_ADDRSTRLEN
#define BUFFER_SIZE 1024

#define CONFIG_FILE "config.txt"
#define ACCOUNT_FILE "account.txt"
#define INDEX_FILE "index.txt"

#define SERVER_PORT 5000
#define BACKLOG 10


// --- MESSAGE TYPES ---
// Auth
#define MSG_REGISTER_REQ                1
#define MSG_REGISTER_RES                2
#define MSG_LOGIN_REQ                   3
#define MSG_LOGIN_RES                   4
#define MSG_LOGOUT_REQ                  5
#define MSG_LOGOUT_RES                  6
#define MSG_SEND_CLIENT_INFO_REQ        7
#define MSG_SEND_CLIENT_INFO_RES        8

// Client 
#define MSG_REGISTER_P2P_POR_REQ        9
#define MSG_REGISTER_P2P_POR_RES        10

// File
#define MSG_SHARE_FILE_REQ              11
#define MSG_SHARE_FILE_RES              12
#define MSG_UNSHARE_FILE_REQ            13
#define MSG_UNSHARE_FILE_RES            14
#define MSG_SEARCH_FILE_REQ             15
#define MSG_SEARCH_FILE_RES             16
// Download
#define MSG_P2P_DOWNLOAD_REQ            17
#define MSG_P2P_DOWNLOAD_RES            18
#define MSG_P2P_FILE_DATA               19
#define MSG_P2P_ERROR                   20

// --- STATUS CODES ---
// Generic codes
#define STATUS_SUCCESS 1
#define STATUS_FAILURE 0

// Authentication code
#define STATUS_ERR_USERNAME_NOTFOUND 10
#define STATUS_ERR_BAD_CREDENTIALS 11
#define STATUS_ERR_USERNAME_ALREADY_TAKEN 12
#define STATUS_ERR_ALREADY_LOGGED_IN 13
#define STATUS_ERR_UNAUTHORIZED 14

// File related code
#define STATUS_ERR_FILE_ALREADY_SHARED 15

#pragma pack(push, 1)
// --- PACKET STRUCTURE ---
// MESSAGE = [HEADER][PAYLOAD]
// -- Header struct --
typedef struct {
    uint8_t msg_type;       // Message type
    uint16_t payload_len;   // Payload length in bytes
} header_t; 

// -- Payload struct --

// 1. Authentication
// Register request
typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} register_req_t;

// Register response
typedef struct {
    uint8_t status;
} register_res_t;

// Login request
typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} login_req_t;

// Login response
typedef struct {
    uint8_t status;
} login_res_t;

// Logout request
typedef struct {
    char username[MAX_USERNAME_LEN];
} logout_req_t;

// Logout response
typedef struct {
    uint8_t status;
} logout_res_t;

// 2. File related

typedef struct {
    uint32_t client_id;
    uint16_t p2p_port;
} register_p2p_port_req_t;

typedef struct {
    uint8_t status; 
    
} register_p2p_port_res_t;

typedef struct {
    uint32_t client_id;
    char file_name[MAX_FILENAME_LEN];
} share_file_req_t;

typedef struct {
    uint8_t status; 
} share_file_res_t;

#pragma pack(pop);

#endif