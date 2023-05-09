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

/*
typedef struct {
  void *start, *end;
} Area;
*/

/*
struct Context {
  uintptr_t gpr[32], mcause, mstatus, mepc;
  void *pdir;
};
*/
// 栈向低地址生长
// 那么, 对于刚刚加载完的进程, 我们要怎么切换到它来让它运行起来呢?
Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
    Context* cp = kstack.end;
    /* 
    mepc理解为：程序要从这继续运行。 
    因为这是为一个新程序创建/初始化上下文，mepc可理解为：程序从这里【开始】运行。
    */
    cp->mepc = (uintptr_t)entry;
    cp->sp = (uintptr_t)cp;
    return cp;
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
