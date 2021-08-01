#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "io.h"

int main(){
    // int fd = open("foo.txt", O_RDWR, 0);
    char buf[1024];
    rio_t rio_b;
    rio_open(&rio_b, "foo.txt", O_RDWR);
    while(rio_readlineb(&rio_b, buf, 1024) > 0){
        printf("%s", buf);
    }
}