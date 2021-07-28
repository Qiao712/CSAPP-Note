#include "mm.h"
#include "memlib.h"
#include "stdlib.h"
#include <stdio.h>

char* heap_listp;   //堆的首地址

void* extend_heap(size_t size){
    size = size % DSIZE ? size - size%DSIZE + DSIZE : size;
    void* bp;                                       //新分配的堆空间的开始

    //分配空间
    if((bp = mem_sbrk(size)) == -1){
        printf("Error: mm_init() failed.\n");
        return (void*)-1;
    }

    //在新分配的空间建立未分配的块
    PUT(HDRP(bp), PACK(size, 0));         //头部 （覆盖之前的epilogue header结尾块）
    PUT(FTRP(bp), PACK(size, 0));         //脚部
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));  //新的 epilogue header 结尾块

    //若前一个块未分配则与之合并
    return coalesce(bp);                   //返回新的块地址
}

void mm_init(){
    mem_init();
    
    //申请4个字作为初始
    if((heap_listp = mem_sbrk(4*WSIZE)) == -1){
        printf("Error: mm_init() failed.\n");
        exit(0);
    }
    PUT(heap_listp, 0);                         //用于对齐
    //分配序言块（第一个块,标记为已分配,大小为最小的情况8字)
    PUT(heap_listp + WSIZE, PACK(DSIZE,1));     //头部
    PUT(heap_listp + 2*WSIZE, PACK(DSIZE,1));   //脚部
    PUT(heap_listp + 3*WSIZE, PACK(0,1));       //epilogue header 结尾块(特殊标记：大小为0（特例），只有头部，没有脚部)

    extend_heap(CHUNKSIZE);
}

static void* coalesce(void* bp){
    //查看前后是否已分配
    char prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    char next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

    char* new_hdrp,* new_ftrp;
    size_t size;
    if(prev_alloc && next_alloc){           //前后的块都被分配
        return bp;
    }else if(prev_alloc){                   //前面的块被分配
        new_hdrp = HDRP(bp);
        new_ftrp = FTRP(NEXT_BLKP(bp));
        size = GET_SIZE(new_ftrp) + GET_SIZE(new_hdrp);
    }else if(next_alloc){                   //后面的块被分配
        new_hdrp = HDRP(PREV_BLKP(bp));
        new_ftrp = FTRP(bp);
        size = GET_SIZE(new_ftrp) + GET_SIZE(new_hdrp);
    }else{                                  //都未分配
        new_hdrp = HDRP(PREV_BLKP(bp));
        new_ftrp = FTRP(NEXT_BLKP(bp));
        size = GET_SIZE(new_ftrp) + GET_SIZE(new_hdrp) + GET_SIZE(HDRP(bp));
    }
    PUT(new_hdrp, PACK(size, 0));
    PUT(new_ftrp, PACK(size, 0));

    return new_hdrp + WSIZE;
}

void mm_free(void* bp){
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));   //清除分配位
    PUT(FTRP(bp), PACK(size, 0));   //清除分配位
    coalesce(bp);
}

void* mm_malloc(size_t size){
    if(size == 0) return NULL;
    
    //双字对齐
    size += DSIZE;  //头部和脚部的大小
    size = size % DSIZE ? size - size%DSIZE + DSIZE : size;

    char* bp;
    while(1){
        if((bp = find_fit(size)) != NULL){
            size_t blk_size = GET_SIZE(HDRP(bp));
            printf("malloc %d byte(s) %d\n", size, blk_size);
            PUT(HDRP(bp), PACK(size, 1));               //设置头部
            PUT(FTRP(bp), PACK(size, 1));               //设置脚部
            if(size != blk_size){               //若没有占满整个块
                PUT(HDRP(NEXT_BLKP(bp)), PACK(blk_size - size, 0));
                PUT(FTRP(NEXT_BLKP(bp)), PACK(blk_size - size, 0));
            }
            return bp;
        }else{
            //分配失败，尝试扩大堆
            if(extend_heap(CHUNKSIZE) == -1){
                //无法在扩大堆，则报错
                printf("Error: mm_malloc() failed.\n");
                return NULL;
            }
        }
    }
}

//first fit
static void* find_fit(size_t size){
    void* bp = heap_listp + DSIZE;  //指向第一个块

    while(1){
        int blk_size = GET_SIZE(HDRP(bp));
        if(blk_size == 0) return NULL;  //堆结束
        if(blk_size >= size && GET_ALLOC(HDRP(bp)) == 0) return bp;   //已找到
        bp = NEXT_BLKP(bp);
    }
}

//打印块分配情况
void mm_print(){
    printf("0x%p: heap begin\n", heap_listp);
    void* bp = heap_listp + DSIZE;
    while(1){
        if(GET_SIZE(HDRP(bp)) == 0){
            printf("0x%p: epilogue header.\n", HDRP(bp));
            return;
        }
        printf("0x%p: header %d byte(s) %d\n", HDRP(bp), GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)));
        printf("0x%p: footer %d byte(s) %d\n", FTRP(bp), GET_SIZE(FTRP(bp)), GET_ALLOC(FTRP(bp)));
        bp = NEXT_BLKP(bp);
    }
}