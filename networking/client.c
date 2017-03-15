#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

#include "client.h"
#include "network_helpers.c"

int connect_to_server(char *IP, char *port) 
{
    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* IPv4 only */
    hints.ai_socktype = SOCK_STREAM; /* TCP */

    s = getaddrinfo(IP, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
	return -1;
    }

    if (connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
        return -1;
    }
    
    freeaddrinfo(result);

    return sock_fd;
}

int send_to_server(int sock_fd, char *msg) {
    return send_to_peer(sock_fd, msg);
}


char *listen_to_server(int sock_fd) {
    return listen_to_peer(sock_fd);
}

void close_client(int fp)
{
	close(fp);
}
