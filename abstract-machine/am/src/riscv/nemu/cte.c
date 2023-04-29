#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

// ��Բ�ͬ��ϵ�ṹ
// �л�ִ������ԭ��ͬ�����Ҫ�����л�ԭ��
// ��ȻҪ���л�������Ҫ���浱ǰ���̵�״̬��ֻ��ͨ�üĴ����ѷ����ص������������ֻ�豣��Ĵ����ѵ�����
// ����ͬ��ϵ�ṹ�ԼĴ����ԵĹ涨��ͬ�����Ҫ����Ĵ�����

// �Ӵ����������쳣�����������¼��ķַ�
Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
        // ���ʵ��mepc+4��������ô��ǿ��Ը��ݲ�ͬ�쳣�����Ƿ�+4?
    case 11:
        if (c->GPR1 == -1) { ev.event = EVENT_YIELD; c->mepc += 4; break; }
        ev.event = EVENT_SYSCALL; c->mepc += 4; break;
    default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
  return c;
}

extern void __am_asm_trap(void);

// ��������ָ��, �û�����Ϳ��Խ�ִ�����л�������ϵͳ��ָ������ڡ��ˡ�
// �û�����������������٣�������ֻ��ϵͳ���ô��Ρ�

bool cte_init(Context*(*handler)(Event, Context*)) {
    // initialize exception entry
    asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));
    asm volatile("li t0, %0" : : "i"(0x0000000a00001800));
    asm volatile("csrw mstatus, t0");

    // register event handler
    user_handler = handler;

    return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
    return NULL;
}

void yield() {
    // asm volatile("li a7, -1; ecall");
    asm volatile("li a7, -1");
    asm volatile("ecall");
}

bool ienabled() {
    return false;
}

void iset(bool enable) {
}
