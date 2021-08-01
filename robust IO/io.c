#include "io.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int rio_readn(int fd, void* usrbuf, size_t n){
    size_t nread;            //已读字节数
    size_t nrest = n;        //剩余字节数
    char* bufp = usrbuf;     //缓冲区指针

    while(nrest > 0){
        if((nread = read(fd, bufp, nrest)) == 0){
            break;       //EOF
        }else if(nread < 0){
            if(errno == EINTR){
                nread = 0;          //Interrepted by sig
            }else{
                return -1;
            }
        }

        nrest -= nread;
        bufp += nread;
    }

    return (n - nrest);    //return num of read character
}

int rio_writen(int fd, void* usrbuf, size_t n){
    size_t nwritten;
    size_t nrest = n;
    char* bufp = usrbuf;

    while(nrest > 0){
        if((nwritten = write(fd, bufp, nrest)) <= 0){
            if(errno == EINTR){
                nwritten = 0;
            }else{
                return -1;
            }
        }

        bufp += nwritten;
        nrest -= nwritten;
    }

    return n;   //promise to writen n character
}

int rio_open(rio_t* rp, const char* file, int flag){
    rio_readinitb(rp, open(file, flag));
}

int rio_readinitb(rio_t* rp, int fd){
    rp->fd = fd;
    rp->cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

int rio_read(rio_t* rp, char* usrbuf, size_t n){
    //如果缓冲区为空，则尝试调用read读取
    while(rp->cnt <= 0){
        rp->cnt = read(rp->fd, rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->cnt<0){              //错误
            if(errno != EINTR){
                return -1;   
            }
        }else if(rp->cnt == 0){     //EOF
            return 0;
        }else{                      //success, reset rio_bufptr
            rp->rio_bufptr = rp->rio_buf;
        }
    }

    //缓冲区不为空，将内容复制到usrbuf
    size_t min_size = n < rp->cnt ? n : rp->cnt;
    memcpy(usrbuf, rp->rio_bufptr, min_size);
    rp->cnt -= min_size;
    rp->rio_bufptr += min_size;
    return min_size;
}

int rio_readlineb(rio_t* rp, void* usrbuf, size_t n){
    int rc, i;
    char* bufptr = usrbuf;
    for(i = 1; i<n; i++){       //最多读n-1个字符
        rc = rio_read(rp, bufptr, 1);
        
        if(rc == 1){        //成功读取一个字符
            bufptr++;
            if(*(bufptr-1) == '\n'){
                break;
            }
        }else if(rc == 0){  //EOF
            i--;
            break;
        }else{              //Error
            return -1;
        }
    }

    *bufptr = 0;
    return i;
}

int rio_readnb(rio_t* rp, void* usrbuf, size_t n){
    size_t nread;            //已读字节数
    size_t nrest = n;        //剩余字节数
    char* bufp = usrbuf;     //缓冲区指针

    while(nrest > 0){
        if((nread = rio_read(rp, bufp, nrest)) == 0){
            break;       //EOF
        }else if(nread < 0){
            return -1;
        }

        nrest -= nread;
        bufp += nread;
    }

    return (n - nrest);    //return num of read character
}