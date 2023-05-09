#ifndef __PROC_H__
#define __PROC_H__

#include <common.h>
#include <memory.h>

// #define PGSIZE 4096
#define STACK_SIZE (8 * PGSIZE)  // 32KB

typedef struct {
    uint8_t stack[STACK_SIZE] PG_ALIGN;
    struct {
        Context *cp;
        AddrSpace as;
        // we do not free memory, so use `max_brk' to determine when to call _map()
        uintptr_t max_brk;
    } info ;
} PCB;

/*
PCB里有一个栈？
和内核线程不同，用户进程的代码，数据和堆栈都应该位于用户区，而且需要保证用户进程能且只能访问自己的代码，数据和堆栈。
*/

extern PCB *current;

#endif
