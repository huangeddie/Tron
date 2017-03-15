
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <stdio.h>

#include "../constants.h"

int sendall(int s, const char *buf, int len) {
    int total = 0;        // how many bytes we've sent
    int bytesleft = len; // how many we have left to send
    int n;
    
    while(total < len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { return -1; }
        total += n;
        bytesleft -= n;
    }
    
    return 0; // return -1 on failure, 0 on success
}

int recvall(int s, char *buf, int len) {
    int total = 0;
    int bytesleft = len;
    int n;
    
    while (total < len) {
        n = recv(s, buf + total, bytesleft, 0);
        if (n == -1) { return -1; }
        total += n;
        bytesleft -=n;
    }
    
    return 0; // return -1 on failure, 0 on success
}

int send_to_peer(int sock_fd, const char *msg) {
    assert(sizeof(size_t) + strlen(msg) + 1 < MAX_BYTES);
    
    size_t num_bytes = strlen(msg) + 1;
    
    //header
    if (sendall(sock_fd, (const char*)&num_bytes, sizeof(size_t)) == -1) {
        fprintf(stderr, "Failed to send header to server");
        return -1;
    }
    
    //data
    if (sendall(sock_fd, msg, num_bytes) == -1) {
        fprintf(stderr, "Failed to send data to server");
        return -1;
    }
    
    return 0; // return -1 on failure, 0 on success
}

char *listen_to_peer(int sock_fd) {
    static char buf[MAX_BYTES];
    int n;
    if ( (n = recv(sock_fd, buf, sizeof(size_t), MSG_DONTWAIT)) == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return NULL;
        } else {
            perror("recv()");
            return NULL;
        }
    }
    if (n == 0) {
        perror("recv()");
        exit(0);
    }
    assert(n <= sizeof(size_t));
    //ensure we get reset of header
    if (recvall(sock_fd, buf + n, sizeof(size_t) - n) == -1) {
        fprintf(stderr, "Error: recvall()");
        return NULL;
    }
    
    //get num_bytes that we're going to hear from
    size_t num_bytes = *(size_t*)buf;
    if (recvall(sock_fd, buf + sizeof(size_t), num_bytes) == -1) {
        fprintf(stderr, "Error: Could not receive all necessary bytes");
        return NULL;
    }
    
    return buf + sizeof(size_t);
}
