#include "wrapped_socket/wrapped_socket.h"
#include "robust_IO_copy/io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
    if(argc != 3){
        fprintf(stderr,"Usage: %s <hostname> <port>", argv[0]);
        return -1;
    }

    int fd = open_clientfd(argv[1], argv[2]);   //open cliend socket connect to a server
    if(fd == -1){
        printf("Can't connect.\n");
        fflush(stdout);
        return -1;
    }
    rio_t rio;
    rio_readinitb(&rio, fd);                    //init rio's read with buffer

    char buf[10240];
    while(fgets(buf, 10240, stdin) != NULL){
        int n = rio_writen(fd, buf, strlen(buf));               //send to server
        fprintf(stdout, "Send %d bytes.\n", n);
        rio_readlineb(&rio, buf, 10240);                        //read message from server
        fprintf(stdout, "Server respones: %s", buf);
        fflush(stdout);
    }

    close(fd);
}