#include <stdlib.h>
#include "memlib.h"

/*  简单的分配器 malloc       */
/*  双字节对齐的、带头部和脚部 */

#define WSIZE (4)     //字长度 （块头部和脚步长度）(bytes)
#define DSIZE (8)     //双字长度(bytes)
#define CHUNKSIZE (1<<10)   //堆增长的步长(bytes)

#define MAX(x,y) ((x>y) ? (x) : (y))

//将长度和分配位合并到一个字
#define PACK(size, alloc) ((size) | (alloc))

//在p地址处读/写一个字, 避免反复的强制类型转换
#define GET(p) (*(unsigned int*)(p))
#define PUT(p,x) ((*(unsigned int*)(p)) = (x))

//对头/脚部的操作， p为头部地址
#define GET_SIZE(p) (GET(p) & ~0x7)    //去除低三位，获取块长度
#define GET_ALLOC(p) (GET(p) & 0x1)    //查看最低位，1：已经分配 0：未分配

//获取头部/脚部地址，bp为块地址（有效负荷首地址）
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

//获取上/下一个相邻块的地址（有效负荷首地址），bp为块地址（有效负荷首地址）
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE((char*)(bp) - WSIZE))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))


void* extend_heap(size_t size);   //扩展堆
static void* coalesce(void *bp);         //合并块
static void* find_fit(size_t size);      //找到大小合适的块
void mm_init();                          //初始化
void* mm_malloc(size_t size);
void mm_free(void* p);

//打印块分配情况
void mm_print();