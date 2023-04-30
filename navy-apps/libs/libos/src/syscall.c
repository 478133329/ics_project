#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "syscall.h"

// helper macros
#define _concat(x, y) x ## y
#define concat(x, y) _concat(x, y)
#define _args(n, list) concat(_arg, n) list
#define _arg0(a0, ...) a0
#define _arg1(a0, a1, ...) a1
#define _arg2(a0, a1, a2, ...) a2
#define _arg3(a0, a1, a2, a3, ...) a3
#define _arg4(a0, a1, a2, a3, a4, ...) a4
#define _arg5(a0, a1, a2, a3, a4, a5, ...) a5

// extract an argument from the macro array
#define SYSCALL  _args(0, ARGS_ARRAY)
#define GPR1 _args(1, ARGS_ARRAY)
#define GPR2 _args(2, ARGS_ARRAY)
#define GPR3 _args(3, ARGS_ARRAY)
#define GPR4 _args(4, ARGS_ARRAY)
#define GPRx _args(5, ARGS_ARRAY)

// ISA-depedent definitions
#if defined(__ISA_X86__)
# define ARGS_ARRAY ("int $0x80", "eax", "ebx", "ecx", "edx", "eax")
#elif defined(__ISA_MIPS32__)
# define ARGS_ARRAY ("syscall", "v0", "a0", "a1", "a2", "v0")
#elif defined(__ISA_RISCV32__) || defined(__ISA_RISCV64__)
# define ARGS_ARRAY ("ecall", "a7", "a0", "a1", "a2", "a0")
#elif defined(__ISA_AM_NATIVE__)
# define ARGS_ARRAY ("call *0x100000", "rdi", "rsi", "rdx", "rcx", "rax")
#elif defined(__ISA_X86_64__)
# define ARGS_ARRAY ("int $0x80", "rdi", "rsi", "rdx", "rcx", "rax")
#elif defined(__ISA_LOONGARCH32R__)
# define ARGS_ARRAY ("syscall 0", "a7", "a0", "a1", "a2", "a0")
#else
#error _syscall_ is not implemented
#endif

intptr_t _syscall_(intptr_t type, intptr_t a0, intptr_t a1, intptr_t a2) {
	// 自陷之前，将系统调用号和参数保存到规定的寄存器中。
	register intptr_t _gpr1 asm (GPR1) = type;
	// 声明一个类型为intptr_t的变量_gpr1，register表示优先存入寄存器中，asm(GPR1)表示该变量和GPR1建立关系，=type为初始化。
	register intptr_t _gpr2 asm (GPR2) = a0;
	register intptr_t _gpr3 asm (GPR3) = a1;
	register intptr_t _gpr4 asm (GPR4) = a2;
	register intptr_t ret asm (GPRx);
	asm volatile (SYSCALL : "=r" (ret) : "r"(_gpr1), "r"(_gpr2), "r"(_gpr3), "r"(_gpr4));
	return ret;
}

/* chatgpt linux直接向用户空间提供write接口。
ssize_t write(int fd, const void *buf, size_t count) {
	return syscall(SYS_write, fd, buf, count);
}
*/

void _exit(int status) {
  _syscall_(SYS_exit, status, 0, 0);
  while (1);
}

int _open(const char *path, int flags, mode_t mode) {
  _exit(SYS_open);
  return 0;
}

// Newlib C 库中的 write 函数最终会调用 _write 函数，write -> _write_r -> _write。
int _write(int fd, void *buf, size_t count) {
	int ret = _syscall_(SYS_write, fd, buf, count);
	return ret;
}

// 堆区的使用情况是由libc来进行管理的，但堆区的大小却需要通过系统调用向操作系统提出更改。
// 在Navy的Newlib中，sbrk()最终会调用_sbrk()。
/*
批处理(batching)的技术：将一些简单的任务累积起来，然后再一次性进行处理，【缓冲区】是批处理技术的核心。
libc中的fread()和fwrite()正是通过缓冲区来将数据累积起来，然后再通过一次系统调用进行处理。
例如通过一个1024字节的缓冲区，就可以通过一次系统调用直接输出1024个字符，而不需要通过1024次系统调用来逐个字符地输出。
*/
extern uint8_t _end;
static uint8_t* old_break = &_end;
void *_sbrk(intptr_t increment) {
	uint8_t* new_break = &_end + increment;
	int ret = _syscall_(SYS_brk, increment, 0, 0);
	if (ret == 0) {
		uint8_t* res = old_break;
		old_break = new_break;
		return (void*)res;
	}
	return (void *)-1;
}

int _read(int fd, void *buf, size_t count) {
	int ret = _syscall_(SYS_read, fd, buf, count);
	return ret;
}

int _close(int fd) {
	int ret = _syscall_(SYS_close, fd, 0, 0);
	return ret;
}

off_t _lseek(int fd, off_t offset, int whence) {
	int ret = _syscall_(SYS_read, fd, offset, whence);
	return ret;
}

int _gettimeofday(struct timeval *tv, struct timezone *tz) {
  _exit(SYS_gettimeofday);
  return 0;
}

int _execve(const char *fname, char * const argv[], char *const envp[]) {
  _exit(SYS_execve);
  return 0;
}

// Syscalls below are not used in Nanos-lite.
// But to pass linking, they are defined as dummy functions.

int _fstat(int fd, struct stat *buf) {
  return -1;
}

int _stat(const char *fname, struct stat *buf) {
  assert(0);
  return -1;
}

int _kill(int pid, int sig) {
  _exit(-SYS_kill);
  return -1;
}

pid_t _getpid() {
  _exit(-SYS_getpid);
  return 1;
}

pid_t _fork() {
  assert(0);
  return -1;
}

pid_t vfork() {
  assert(0);
  return -1;
}

int _link(const char *d, const char *n) {
  assert(0);
  return -1;
}

int _unlink(const char *n) {
  assert(0);
  return -1;
}

pid_t _wait(int *status) {
  assert(0);
  return -1;
}

clock_t _times(void *buf) {
  assert(0);
  return 0;
}

int pipe(int pipefd[2]) {
  assert(0);
  return -1;
}

int dup(int oldfd) {
  assert(0);
  return -1;
}

int dup2(int oldfd, int newfd) {
  return -1;
}

unsigned int sleep(unsigned int seconds) {
  assert(0);
  return -1;
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
  assert(0);
  return -1;
}

int symlink(const char *target, const char *linkpath) {
  assert(0);
  return -1;
}

int ioctl(int fd, unsigned long request, ...) {
  return -1;
}
