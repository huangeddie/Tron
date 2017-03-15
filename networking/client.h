
#include "../constants.h"

#define IP_ADDR "172.22.155.23"
#define PORT "1234"

int connect_to_server(char *IP, char *port);

//msg must be null-terminated. Length of msg must also be within MAX_BYTES
int send_to_server(int sock_fd, char* msg);

//returns a pointer to static memory so do not deallocate
char *listen_to_server(int sock_fd);

void close_client(int fp);
