#include <stddef.h>

int rio_readn(int fd, void* usrbuf, size_t n);          //robust reading without buffer
int rio_writen(int fd, void* usrbuf, size_t n);         //robust writing without buffer

//with buffer
#define MAX_BUFSIZE 8192
typedef struct{
    int fd;
    int cnt;
    char* rio_bufptr;
    char rio_buf[MAX_BUFSIZE];
}rio_t;
int rio_readinitb(rio_t* rp, int fd);
int rio_open(rio_t* rp, const char* file, int flag);
static int rio_read(rio_t* rp, char* usrbuf, size_t n);         //带缓冲区的读取
int rio_readlineb(rio_t* rp, void* usrbuf, size_t n);           //读取一行，包括换行符，以空字符结束
int rio_readnb(rio_t* rp, void* usrbuf, size_t n);              //