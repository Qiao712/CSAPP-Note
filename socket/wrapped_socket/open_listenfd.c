#include "wrapped_socket.h"
#include <stddef.h>
#include <string.h>
#include <unistd.h>

int open_listenfd(char* port){
    //set hints
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV;   //listen | on any IP address | using port number
    
    //walk the list to try to bind
    struct addrinfo *listp, *p;
    int fd, rc;
    int optval = 1; //?
    getaddrinfo(NULL, port, &hints, &listp);
    for(p = listp; p != NULL; p = p->ai_next){
        if((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) continue; //socket failed
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
        if((rc = bind(fd, p->ai_addr, p->ai_addrlen)) == 0) break;    //success
        close(fd);
    }

    freeaddrinfo(listp);
    if(p == NULL) return -1; //all failed

    //make it a listening socket ready to accept connection requests
    if(listen(fd, 1024) == -1){
        //failed
        close(fd);
        return -1;
    }
    return fd;
}