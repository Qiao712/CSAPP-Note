#include <sys/socket.h>
#include <netdb.h>

//open client's socket connecting to hostname:port, return fd
int open_clientfd(char* hostname, char* port);
//open server's socket listening on a port, return fd
int open_listenfd(char* port);