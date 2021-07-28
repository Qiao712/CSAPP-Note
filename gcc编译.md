```
graph TB
1(main.c) --> 2[cpp]
2-->3(main.i)
3-->4[cc1]
4-->5(main.s)
5-->6[as]
6-->7(main.o)

a(src1.c) --> b[cpp]
b-->c(src1.i)
c-->d[cc1]
d-->e(src1.s)
e-->f[as]
f-->g(src1.o)

7-->8[ld]
g-->8
8-->可执行文件
```
* cpp(C preprocessor) 预处理程序，生成中间文件
* cc1 C语言编译器，生成汇编文件
* as 汇编程序，生成**可重定位目标文件(relocatable object file)**
* ld 连接器Linker，链接多个可重定位目标文件,生成**可执行目标文件(executable object file)**。
* 编译驱动程序(compiler driver)自动完成以上步骤

参考:《CSAPP》 Link