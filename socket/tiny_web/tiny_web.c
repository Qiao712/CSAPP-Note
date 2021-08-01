#include <stdio.h>
#include <sys/io.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#include "../robust_IO_copy/io.h"
#include "../wrapped_socket/wrapped_socket.h"

const int MAXLINE = 1024;
const char* DEFAULT_FILENAME = "home.html";

void doit(int fd);                      //处理一个http事物
void read_requesthdrs(rio_t* rp);       //读取 请求报头（在此仅丢弃）
int parse_uri(char* uri, char* filename, char* cgiargs);                        //解析URI（统一资源标识符）
void serve_static(int fd, char* filename, size_t file_size);                       //发送包含静态内容的HTTP响应（通过文件后缀判断MIME类型）
void get_filetype(char* filename, char* filetype);                              //根据文件后缀判断文件类型
void serve_dynamic(int fd, char* filename, char* cgiargs);                      //调用CGI程序，发送HTTP响应
void clienterror(int fd, char* cause, char* errnum, char* shortmsg, char* longmsg);
void strupr(char* str);

int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int listenfd = open_listenfd(argv[1]);
    if(listenfd == -1){
        fprintf(stderr, "Error: Can't open listen socket.\n");
        return 1;
    }

    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    int connfd;
    while(1){
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
        if(connfd == -1) continue;

        doit(connfd);
        close(connfd);
    }
}

void doit(int fd){
    rio_t rio;
    rio_readinitb(&rio, fd);
    
    //read request line and parse it
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_readlineb(&rio, buf, MAXLINE);                  //read request line
    sscanf(buf, "%s %s %s", method, uri, version);      //parse
    printf("Request: %s %s %s\n", method, uri, version);

    //only support GET
    if(strcmp(method, "GET") != 0){
        //TODO:
        // clienterror()
        printf("TINY Web only support GET method.\n");
        return;
    }

    //deal with request header(abondan)
    read_requesthdrs(&rio);

    //parse uri
    char filename[MAXLINE], cgiargs[MAXLINE], is_dynamic_content;
    is_dynamic_content = parse_uri(uri, filename, cgiargs);
    if(is_dynamic_content) printf("\n\tResponse dynamic content: %s %s\n", filename, cgiargs);
    else printf("\tResponse static content: %s\n", filename);
    fflush(stdout);

    //check whether the file is existed
    struct stat sbuf;
    if(stat(filename, &sbuf) < 0){
        //no the file, 404
        clienterror(fd, method, "404", "Not Found", "Not Found");
        return;
    }

    //serve
    if(is_dynamic_content){
        //dynamic content
        serve_dynamic(fd, filename, cgiargs);
    }else{
        //static content
        serve_static(fd, filename, sbuf.st_size);
    }
}

void read_requesthdrs(rio_t* rp){
    //just print it
    char buf[MAXLINE];
    int rc;
    while(1){
        if(rio_readlineb(rp, buf, MAXLINE) > 0){
            if(strcmp(buf, "\r\n") == 0) return;
            //do
            printf("\tRequest Header: %s", buf);
        }else{
            return;
        }
    }
}

int parse_uri(char* uri, char* filename, char* cgiargs){
    const char* cigbin_dir = "/cgi-bin/";
    
    if(strstr(uri, cigbin_dir)){
        //cgi-bin -- dynamic content
        int ptr;        // '?' position
        for(ptr = 0; uri[ptr] && uri[ptr] != '?'; ptr++);
        if(uri[ptr] == 0){
            *cgiargs = 0;
        }else{
            strcpy(cgiargs, uri+ptr+1);
            uri[ptr] = 0; //cut
        }

        strcpy(filename, ".");
        strcat(filename, uri);
        return 1;
    }else{
        //static content
        strcpy(filename, ".");
        strcat(filename, uri);
        //check the last character
        if(filename[strlen(filename) - 1] == '/'){
            //add default filename
            strcat(filename, DEFAULT_FILENAME);
        }
        return 0;
    }
}

void serve_static(int fd, char* filename, size_t filesize){
    //get file tpye
    char filetype[MAXLINE];
    get_filetype(filename, filetype);
    printf("\tfilename: %s   filetpye: %s\n",filename, filetype);
    
    //gen header
    char buf[MAXLINE*3];
    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    strcat(buf, "Server: Tiny Web Server\r\n");
    strcat(buf, "Connection: close\r\n");
    sprintf(buf, "%sContent-length: %ld\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n", buf, filetype);
    strcat(buf, "\r\n");

    rio_writen(fd, buf, strlen(buf));

    //read file and write to HTTP response
    int srcfd = open(filename, O_RDONLY, 0);
    char* srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close(srcfd);
    rio_writen(fd, srcp, filesize);
    munmap(srcp, filesize);
}

void clienterror(int fd, char* cause, char* errnum, char* shortmsg, char* longmsg){
    char buf[MAXLINE], content[MAXLINE];
    printf("\tError: %s %s %s\n", cause, errnum, shortmsg);
    fflush(stdout);

    sprintf(content, "<h> Error </h>\n <p> %s </p>", longmsg);

    //send error message to client
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    strcat(buf, "Server: Tiny Web Server\r\n");
    strcat(buf, "Content-type: text/html\r\n");
    sprintf(buf, "%sContent-length: %ld\r\n", buf, strlen(content));
    strcat(buf, "\r\n");

    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, content, strlen(content));
}

void serve_dynamic(int fd, char* filename, char* cgiargs){
    char buf[MAXLINE], *argv[MAXLINE];
    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    strcat(buf, "Server: Tiny Web Server\r\n");
    strcat(buf, "\r\n");
    rio_writen(fd, buf, strlen(buf));
    
    if(fork() == 0){
        //parse cgiargs to argv
        char *i, *j;
        int k = 0;
        for(i = j = cgiargs; *i; i++){
            if(*i == '&'){
                *i = 0; //cut
                argv[k] = malloc(i - j + 1);
                memcpy(argv[k], j, i - j + 1);
                ++k;
                j = i+1;
            }
        }
        if(i != cgiargs){
            argv[k] = malloc(i - j + 1);
            memcpy(argv[k], j, i - j + 1);
            ++k;
        }
        argv[k] = NULL;
        printf("\targc : %d\n", k);

        dup2(fd, STDOUT_FILENO);
        execve(filename, argv, __environ);
    }

    wait(NULL);
}

void get_filetype(char* filename, char* filetype){
    //get last '.'
    char* dot_ptr = NULL;
    for(int i = 0; filename[i]; i++)
        if(filename[i] == '.') dot_ptr = filename + i;
    
    if(dot_ptr == NULL){
        strcpy(filetype, "text/plain");
        return;
    }
    
    char postfix[MAXLINE];
    strcpy(postfix, dot_ptr + 1);
    strupr(postfix);
    if(!strcmp(postfix, "HTML") || !strcmp(postfix, "HML")) strcpy(filetype, "text/html");
    else if(!strcmp(postfix, "JPEG")) strcpy(filetype, "image/jpeg");
    else if(!strcmp(postfix, "GIF")) strcpy(filetype, "image/gif");
    else if(!strcmp(postfix, "PNG")) strcpy(filetype, "image/png");
    else if(!strcmp(postfix, "BMP")) strcpy(filetype, "image/bmp");
    else strcpy(filetype, "text/plain");
}


void strupr(char* str){
    for(int i = 0; str[i]; i++){
        if(str[i] >= 'a' && str[i] <= 'z') str[i] = str[i] - 'a' + 'A';
    }
}