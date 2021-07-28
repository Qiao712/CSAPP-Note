/*用于提供内存模型， 模拟堆*/
#include <stdlib.h>
#include <stdio.h>

#define MAX_HEAP 5*1024*1024
static char* mem_heap;      //模拟堆的起始
static char* mem_brk;       //模拟堆的边界
static char* mem_max_addr;  //最大上界

//初始化内存模型
void mem_init(){
    mem_heap = (char*)malloc(MAX_HEAP);
    if(mem_heap == NULL) printf("Error: mem_init() failed.\n");
    mem_brk = mem_heap;
    mem_max_addr = mem_heap + MAX_HEAP;
}

//模拟sbrk()函数，用于扩展堆，只实现了增大
void* mem_sbrk(unsigned int incr){
    char* old_mem_brk = mem_brk;
    if(incr < 0 || mem_brk + incr > mem_max_addr){
        fprintf(stderr, "ERROR:mem_sbrk failed. Ran out of memory...\n");
        return (void*) -1;
    }
    
    mem_brk += incr;
    return (void*)old_mem_brk;
}