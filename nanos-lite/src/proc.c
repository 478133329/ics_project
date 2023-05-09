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
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void naive_uload(PCB* pcb, const char* filename);
void context_kload(PCB* pcb, void (*entry)(void*), void* arg);


void init_proc() {

    context_kload(&pcb[0], hello_fun, "first thread");
    context_kload(&pcb[1], hello_fun, "second thread");

    switch_boot_pcb();

    Log("Initializing processes...");

    // load program here
    // naive_uload(NULL, "/bin/bmp-test");

}


Context* schedule(Context *prev) {
    // 前面分析，一个进程在保存完上下文后，pcb中的cp实际上和sp指向用一个内存区域。
    // 但是pcb中的cp并不是连续赋值，需要在被切换走的时候，要手动更新。
    current->cp = prev;

    current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
    return current->cp;
}