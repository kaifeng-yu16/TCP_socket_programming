#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <netdb.h>
#include <unistd.h>

// return socket_fd if success, -1 if failed
int client_init(const char * hostname, const char * port);

// return socket_fd if success, -1 if failed
int server_init(const char * port);

// return socket_fd if success, -1 if failed
int accept_client(int socket_fd);
