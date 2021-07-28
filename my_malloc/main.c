#include <stdio.h>
#include "mm.h"
#include "memlib.h"

int main(){
    mm_init();
    mm_print();
    printf("......................\n");
    void* p1 = mm_malloc(1500);
    void* p2 = mm_malloc(100);
    mm_print();
    printf("......................\n");
    mm_free(p2);
    mm_print();
    printf("......................\n");
    mm_malloc(1200);
    mm_print();
    printf("......................\n");
    mm_malloc(6000);
    mm_print();
    printf("......................\n");
    mm_free(p1);
    mm_print();
    // mm_print();
    // printf("......................\n");
    // mm_malloc(2000);
    // mm_print();
    // printf("......................\n");
}