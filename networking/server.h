
// Can select or poll be used to optimize this program?

#include "../constants.h"

#define MAX_CONNECTIONS_EVER 100

#define OTHER_ERROR -10
#define CLIENT_DISCONNECTED -5
#define OK 0

/*
typedef struct {
    int fd_arr[MAX_CONNECTIONS_EVER];
    size_t size;
} client_list;
*/

typedef struct {
	int fd;
	int disconnected;
} client_t;

typedef struct {
    int sock_fd;
    struct addrinfo *result;
} server_t;

//returns NULL if client has nothing to say, or returns a pointer to static char array. Note: do not deallocate the return
const char *listen_to_client(client_t * client);

//returns 0 if successful or -1 if not (maybe set errno appropriately). msg must be null-terminated. The length of msg must also be within MAX_BYTES
int send_to_client(client_t * client, const char *msg);

//starts the server, returns passive socket
server_t *start_server(int port);

// Returns a null-terminated list of strings that can be used as IPs to connect
char ** get_local_ip_addresses();
void free_local_ip_addresses();
//block until n number of connections have been made. returns the head of the client linked list
client_t ** wait_for_connections(server_t *server, size_t n, void (*funct)(client_t ** clients, int num_made, int total));

void remove_connections(client_t ** connections);

void close_server(server_t * server);
