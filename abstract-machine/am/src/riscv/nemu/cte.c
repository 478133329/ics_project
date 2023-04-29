#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

// 针对不同体系结构
// 切换执行流的原因不同，因此要抽象切换原因。
// 既然要求切换，就需要保存当前进程的状态，只有通用寄存器堆发生重叠，因此上下文只需保存寄存器堆的内容
// 而不同体系结构对寄存器对的规定不同，因此要抽象寄存器。

// 从代码来看，异常处理程序仅做事件的分发
Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
        // 软件实现mepc+4放在这里，好处是可以根据不同异常决定是否+4?
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

// 有了自陷指令, 用户程序就可以将执行流切换到操作系统【指定的入口】了。
// 用户程序能做的事情很少，基本上只有系统调用传参。

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
