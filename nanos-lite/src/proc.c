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
    // navy-test�����ִ��ǰ��û����nano-liteһ����ͨ��start.S��ʼ����sp��
    // ��ʱnavy-testʹ�õ����ں�ջ��naive_uload��������һ���Ӻ������á�
    // naive_uload(NULL, "/bin/nslider");

}


Context* schedule(Context *prev) {
    // ��������prevָ�����spָ�룬mv a0, sp
    // ǰ�������һ�������ڱ����������ĺ�pcb�е�cpʵ���Ϻ�spָ����һ���ڴ�����
    // ����pcb�е�cp������������ֵ����Ҫ�ڱ��л��ߵ�ʱ��Ҫ�ֶ����¡�
    current->cp = prev;
    // ����ǽ��̴�����ĵ�һ���л���cpԭ��ָ���ں�ջ������ָ�����û�ջ

    current = (current == &pcb[0] ? &pcb[0] : &pcb[0]);
    return current->cp;
}