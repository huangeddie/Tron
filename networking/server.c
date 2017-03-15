#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>

#include "server.h"
#include "network_helpers.c"

// Can select or poll be used to optimize this program?

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int hit_sigpipe;

void sigpipe_handler(int signo) 
{
    hit_sigpipe = 1;
}

const char * listen_to_client(client_t * client) 
{
    return listen_to_peer(client->fd);
}

int send_to_client(client_t * client, const char * msg)
{
    return send_to_peer(client->fd, msg);
}


server_t *start_server(int port)
{
    assert(port > 1024 && port <= 9999);
    char port_str[5];
    sprintf(port_str, "%d", port);
    
    int s;
    int optval = 1;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    s = getaddrinfo(NULL, port_str, &hints, &result);
    if (s != 0) 
	{
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        //exit(1);
	return NULL;
    }
    if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind()");
        return NULL;
    }
    
    server_t *server = malloc(sizeof(server_t));
    server->sock_fd = sock_fd;
    server->result = result;
    
    return server;
}

char ** get_local_ip_addresses() 
{
	char ** output = calloc(sizeof(char*), 10);

    int required_family = AF_INET; // Change to AF_INET6 for IPv6
    struct ifaddrs *myaddrs, *ifa;
    getifaddrs(&myaddrs);
    char host[256], port[256];
    //puts("Available IP addresses:");
	int index = 0;
    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        int family = ifa->ifa_addr->sa_family;
        if (family == required_family && ifa->ifa_addr) {
            if (0 == getnameinfo(ifa->ifa_addr,
                                 (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                 sizeof(struct sockaddr_in6),
                                 host, sizeof(host), port, sizeof(port)
                                 , NI_NUMERICHOST | NI_NUMERICSERV  ))
	    {
		char * ip_addr = calloc(strlen(host) + 1,1);
		strcpy(ip_addr, host);
                output[index] = ip_addr;
		index++;
	    }  
      }
    }
	return output;
}

void free_local_ip_addresses(char ** values)
{
	int i = 0;
	while(values[i])
	{
		free(values[i]);
		i++;
	}
	free(values);
}


client_t ** wait_for_connections(server_t *server, size_t n, void (*funct)(client_t ** clients, int num_made, int total)) 
{
	client_t ** clients = calloc(sizeof(client_t*), n + 1);

    //client_list *list = create_client_list();
    
    if (listen(server->sock_fd, n) != 0) 
    {
        perror("listen()");
        return NULL;
    }
    //struct sockaddr_in *result_addr = (struct sockaddr_in *) server->result->ai_addr;
    //printf("Listening on file descriptor %d, port %d\n", server->sock_fd, ntohs(result_addr->sin_port));
    
    //printf("Waiting for %lu connection(s)...\n", n);
    for (int i = 0; i < n; i++) 
    {
        int client_fd = accept(server->sock_fd, NULL, NULL);
        client_t * newClient = malloc(sizeof(client_t));
	newClient->fd = client_fd;
	newClient->disconnected = 0;
	clients[i] = newClient;
        //add_client(list, client_fd);

	if(funct)
		(*funct)(clients, i+1, n);
    }
    clients[n] = NULL;
    return clients;
    //return list;
}

void remove_connections(client_t ** connections)
{
	int i = 0;
	while(1)
	{
		if(!connections[i]) break;

		close(connections[i]->fd);
		free(connections[i]);

		i++;
	}
	free(connections);
}

void close_server(server_t * server)
{
	close(server->sock_fd);	
	freeaddrinfo(server->result);
	free(server);
}

