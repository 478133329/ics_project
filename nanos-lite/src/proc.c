#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

// ���Ժ�����Ϊһ������
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
    // ǰ�������һ�������ڱ����������ĺ�pcb�е�cpʵ���Ϻ�spָ����һ���ڴ�����
    // ����pcb�е�cp������������ֵ����Ҫ�ڱ��л��ߵ�ʱ��Ҫ�ֶ����¡�
    current->cp = prev;

    current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
    return current->cp;
}