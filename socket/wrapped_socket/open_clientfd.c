#include "wrapped_socket.h"
#include <stddef.h>
#include <string.h>
#include <unistd.h>

int open_clientfd(char* hostname, char* port){
    //set hints
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;        //open a connection
    hints.ai_flags = AI_NUMERICSERV;        //numeric port number
    hints.ai_flags |= AI_ADDRCONFIG;        

    //walk the list to try to connec
    struct addrinfo *listp, *p;
    int fd;
    getaddrinfo(hostname, port, &hints, &listp);
    for(p = listp; p != NULL; p = p->ai_next){
        if((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
            //socket failed
            continue;
        }
        
        int rc;
        if((rc = connect(fd, p->ai_addr, p->ai_addrlen)) != -1) break;  //if success then break

        close(rc);  //connecting failed, close and continue
    }

    freeaddrinfo(listp);
    if(p == NULL) return -1;    //all connects failed
    return fd;
}