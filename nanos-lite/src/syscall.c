#include <common.h>
#include <syscall.h>

#include <fs.h>

#include <sys/time.h>

// 在Linux内核中，系统调用处理程序的命名通常遵循"sys_"前缀的命名约定，如sys_write、sys_read等。

typedef uint32_t mode_t;
typedef int64_t off_t;

void sys_open(Context* c) {
    // (const char *path, int flags, mode_t mode)
    const char* path = (const char*)(c->GPR2);
    int flags = c->GPR3;
    int mode = c->GPR4;
    int ret = fs_open(path, flags, mode);
    c->GPRx = ret;
}

void sys_read(Context* c) {
    // (int fd, void *buf, size_t count)
    int fd = c->GPR2;
    void* buf = (void*)(c->GPR3);
    size_t count = c->GPR4;
    int ret = fs_read(fd, buf, count);
    c->GPRx = ret;
}

void sys_write(Context* c) {
    //(int fd, void *buf, size_t count)
    int fd = c->GPR2;
    // 指针转换为整数通常会产生警告，而整数转换为指针则可能会导致编译错误或者运行时错误。
    // 指针转换为整数，通常转换为uintptr_t，因为针对不同平台进行了条件编译。
    // 如果您必须进行指针到整数的转换，则应该使用显式的类型转换。
    void* buf = (void*)c->GPR3;
    size_t count = c->GPR4;
    /*
    if (fd == 1 || fd == 2) {
        int i = 0;
        for (i = 0; i < count; i++) {
            putch(((char*)buf)[i]);
        }
        c->GPRx = i;
    } else 
    */
    int ret = fs_write(fd, buf, count);
    c->GPRx = ret;
}

void sys_close(Context* c) {
    // (int fd)
    int fd = c->GPR2;
    int ret = fs_close(fd);
    c->GPRx = ret;
}

void sys_lseek(Context* c) {
    // (int fd, off_t offset, int whence)
    int fd = c->GPR2;
    off_t offset = c->GPR3;
    int whence = c->GPR4;
    int ret = fs_lseek(fd, offset, whence);
    c->GPRx = ret;
}

void sys_brk(Context* c) {
    c->GPRx = 0;
}

void sys_gettimeofday(Context* c) {
    // (struct timeval* tv, struct timezone* tz)
    // Warning：返回一个指向内核空间的指针。 如何解决？将用户空间的指针传过来。
    struct timeval* tv = (struct timeval*)c->GPR2;
    AM_TIMER_REALTIME_T time = io_read(AM_TIMER_REALTIME);
    tv->tv_sec = time.us / 1000000;
    tv->tv_usec = time.us;
    c->GPRx = 0;
}

void do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    Log("syscall %d ", a[0]);
    switch (a[0]) {
    case SYS_exit: halt(0);
    case SYS_yield: yield(); break;
    case SYS_open: sys_open(c); break;
    case SYS_read: sys_read(c); break;
    case SYS_write: sys_write(c); break;
    case SYS_close: sys_close(c); break;
    case SYS_lseek: sys_lseek(c); break;
    case SYS_brk: sys_brk(c); break;
    case SYS_gettimeofday: sys_gettimeofday(c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
    }
}
