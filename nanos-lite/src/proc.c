#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

// 测试函数作为一个程序
void hello_fun(void *arg) {
  // int j = 1;
  while (1) {
    // Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    // j ++;
    // yield();
  }
}

void naive_uload(PCB* pcb, const char* filename);
void context_kload(PCB* pcb, void (*entry)(void*), void* arg);
void context_uload(PCB* pcb, const char* filename, char* const argv[], char* const envp[]);

void init_proc() {

    context_kload(&pcb[1], hello_fun, "first thread");
    // context_kload(&pcb[1], hello_fun, "second thread");
    // context_uload(&pcb[0], "/bin/hello");
    // char* const argv[] = { "wang" , NULL };
    // char* const envp[] = { "hello", "world" };
    context_uload(&pcb[0], "/bin/nplayer", NULL, NULL);

    switch_boot_pcb();

    Log("Initializing processes...");

    // load program here
    // 
    // navy-test程序的执行前并没有像nano-lite一样，通过start.S初始化了sp。
    // 此时navy-test使用的是内核栈，naive_uload，更像是一个子函数调用。
    // naive_uload(NULL, "/bin/nslider");

}


Context* schedule(Context *prev) {
    // 传过来的prev指针就是sp指针，mv a0, sp
    // 前面分析，一个进程在保存完上下文后，pcb中的cp实际上和sp指向用一个内存区域。
    // 但是pcb中的cp并不是连续赋值，需要在被切换走的时候，要手动更新。
    current->cp = prev;
    // 如果是进程创建后的第一次切换，cp原本指向内核栈，现在指向了用户栈

    current = (current == &pcb[0] ? &pcb[0] : &pcb[0]);
    return current->cp;
}