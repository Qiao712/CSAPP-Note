## 根据《CSAPP》第11章，编写的socket实验代码
## 系统： Linux
* hostinfo/hostinfo.c 解析主机名，显示ip地址
* robust_IO_copy/ （同./robust IO/)RIO包
* wrapped_socket/ 包装的socket接口
* client.c 客户端测试
```shell
gcc client.c robust_IO_copy/io.c wrapped_socket/open_clientfd.c -o client
```
* server.c 服务器端测试
```shell
gcc server.c robust_IO_copy/io.c wrapped_socket/open_listenfd.c -o server
```