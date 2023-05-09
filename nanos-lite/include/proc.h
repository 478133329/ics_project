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
PCB����һ��ջ��
���ں��̲߳�ͬ���û����̵Ĵ��룬���ݺͶ�ջ��Ӧ��λ���û�����������Ҫ��֤�û���������ֻ�ܷ����Լ��Ĵ��룬���ݺͶ�ջ��
*/

extern PCB *current;

#endif
