void mem_init();                    //初始化内存模型
void* mem_sbrk(unsigned int incr);  //模拟sbrk()函数，用于扩展堆，只实现了增大