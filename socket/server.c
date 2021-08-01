#include "wrapped_socket/wrapped_socket.h"
#include "robust_IO_copy/io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return -1;
    }

    int listenfd = open_listenfd(argv[1]);    //open listening socket
    char buf[10240];
    while(1){
        struct sockaddr_storage clientaddr;
        socklen_t clientlen = sizeof(clientaddr);
        int connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);   //open connect socket
        if(connfd != -1){
            printf("Connect with a client.\n");
            fflush(stdout);
            
            //successively read from client
            rio_t rio;
            int n;
            rio_readinitb(&rio, connfd);
            while((n = rio_readlineb(&rio, buf, 10240)) != 0){
                printf("recive %d bytes: %s", n, buf);
                fflush(stdout);
                rio_writen(connfd, "Server has recived a message.\n", strlen("Server has recived a message.\n"));
            }
        }

        close(connfd);
        printf("Connect closes.\n");
        fflush(stdout);
    }
}