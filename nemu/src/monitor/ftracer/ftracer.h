#ifndef _FTRACER_H
#define _FTRACER_H

#include <common.h>

typedef struct __FUNC_INFO {
    char name[64];
    vaddr_t start;
    size_t size;
}FUNC_INFO;

FUNC_INFO* check_func(vaddr_t addr);

#endif