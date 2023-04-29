#include <common.h>
#include <syscall.h>

// 在Linux内核中，系统调用处理程序的命名通常遵循"sys_"前缀的命名约定，如sys_write、sys_read等。

void sys_write(Context* c) {
    // fd, buf, count
    int fd = c->GPR2;
    // 指针转换为整数通常会产生警告，而整数转换为指针则可能会导致编译错误或者运行时错误。
    // 指针转换为整数，通常转换为uintptr_t，因为针对不同平台进行了条件编译。
    // 如果您必须进行指针到整数的转换，则应该使用显式的类型转换。
    void* buf = (void*)c->GPR3;
    size_t count = c->GPR4;
    if (fd == 1 || fd == 2) {
        int i = 0;
        for (i = 0; i < count; i++) {
            putch(((char*)buf)[i]);
        }
        c->GPRx = i;
    }
}

void sys_brk(Context* c) {
    c->GPRx = 0;
}

void do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    Log("syscall %d - ", a[0]);
    switch (a[0]) {
    case SYS_exit: halt(0);
    case SYS_yield: yield(); break;
    case SYS_write: sys_write(c); break;
    case SYS_brk: sys_brk(c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
    }
    Log("syscall %d over\n ", a[0]);
}
