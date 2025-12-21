#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include "network_utils.h"
#include "protocol.h"

int send_all(int sockfd, void *data, size_t len) {
    size_t total_sent = 0;
    size_t bytes_left = len;
    int n;
    char *ptr = (char *)data;

    while (total_sent < len) {
        n = send(sockfd, ptr + total_sent, bytes_left, 0);
        if (n == -1) { 
            perror("send() error"); 
            return -1; 
        }
        total_sent += n;
        bytes_left -= n;
    }
    return 0;
}

int recv_all(int sockfd, void *data, size_t len) {
    size_t total_received = 0;
    size_t bytes_left = len;
    int n;
    char *ptr = (char *)data;

    while (total_received < len) {
        n = recv(sockfd, ptr + total_received, bytes_left, 0);
        if (n == -1) { 
            perror("recv() error"); 
            return -1; 
        }
        if (n == 0) return 0; 
        
        total_received += n;
        bytes_left -= n;
    }
    return total_received;
}

int send_message(int sockfd, uint8_t msg_type, void *payload, uint16_t payload_len) {
    header_t header;

    header.msg_type = msg_type;
    header.payload_len = payload_len;

    // send header
    if (send_all(sockfd, &header, sizeof(header_t)) == -1) {
        return -1;
    }
    // send payload
    if (payload_len > 0 && payload != NULL) {
        if (send_all(sockfd, payload, payload_len) == -1) {
            return -1;
        }
    }
    return 0;
}

int recv_message(int sockfd, uint8_t *msg_type, void **payload) {
    header_t header;

    // Receive header
    int ret = recv_all(sockfd, &header, sizeof(header_t));
    if (ret <= 0) return ret; 

    // Extract metadata from header
    *msg_type = header.msg_type;
    
    uint16_t payload_len = header.payload_len;

    // Receive payload
    if (payload_len > 0) {
        *payload = malloc(payload_len + 1); 
        if (*payload == NULL) {
            perror("malloc() error");
            return -1;
        }

        // Receive data
        if (recv_all(sockfd, *payload, payload_len) <= 0) {
            free(*payload); // Free if get error
            *payload = NULL;
            return -1;
        }

        // Append null-terminator 
        ((char*)*payload)[payload_len] = '\0';
    } else {
        *payload = NULL;
    }

    return (int) payload_len;
}