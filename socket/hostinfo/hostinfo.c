#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[]){
    struct addrinfo *listp, *p, hints;

    if(argc != 2){
        printf("Need argment!\n");
        return -1;
    }

    //get a list of addrinfo records
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;          //IPv4
    hints.ai_socktype = SOCK_STREAM;    //connections
    int error;
    if((error = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0){
        printf("%s\n", gai_strerror(error));
        return -1;
    }

    //print the list of IP of the domain name
    char buf[1024];
    for(p = listp; p != NULL; p = p->ai_next){
        getnameinfo(p->ai_addr, p->ai_addrlen, buf, 1024, NULL, 0, NI_NUMERICHOST); //NI_NUMERICHOST : only return numberic IP address
        printf("%s\n", buf);
    }

    freeaddrinfo(listp);

    return 0;
}