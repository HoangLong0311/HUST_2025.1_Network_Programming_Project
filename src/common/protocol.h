#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <arpa/inet.h>

// --- CONSTANTS ---
#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 32
#define MAX_IP_LEN INET_ADDRSTRLEN
#define BUFFER_SIZE 1024
#define CONFIG_FILE "config.txt"    
#define DEFAULT_LISTEN_PORT 9090

// --- MESSAGE TYPES ---
#define MSG_REGISTER_REQ 1   
#define MSG_REGISTER_RES 2   
#define MSG_LOGIN_REQ 3      
#define MSG_LOGIN_RES 4      
#define MSG_LOGOUT_REQ 5     
#define MSG_LOGOUT_RES 6     

// --- STATUS CODES --- 
// Generic codes
#define STATUS_SUCCESS 1          
#define STATUS_FAILURE 0   

// Authentication code
#define STATUS_ERR_USERNAME_NOTFOUND 10 
#define STATUS_ERR_BAD_CREDENTIALS 11   
#define STATUS_ERR_USERNAME_ALREADY_EXIST 12
#define STATUS_ERR_ALREADY_LOGGED_IN 13 

// --- PACKET STRUCTURE ---
// MESSAGE = [HEADER][PAYLOAD]
#pragma pack(push, 1)

// -- Header struct --
typedef struct {
    uint8_t msg_type;   // Message type 
    uint16_t len;       // Payload length in bytes
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
} login_response;

// Logout request
typedef struct {
    char username[MAX_USERNAME_LEN]; 
} logout_req_t;

// Logout response
typedef struct {
    uint8_t status; 
} logout_res_t;

#pragma pack(pop)

#endif