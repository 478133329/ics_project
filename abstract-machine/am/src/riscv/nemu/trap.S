#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define MAP(c, f) c(f)

#if __riscv_xlen == 32
#define LOAD  lw
#define STORE sw
#define XLEN  4
#else
#define LOAD  ld
#define STORE sd
#define XLEN  8
#endif

#define REGS(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9) \
f(10) f(11) f(12) f(13) f(14) f(15) f(16) f(17) f(18) f(19) \
f(20) f(21) f(22) f(23) f(24) f(25) f(26) f(27) f(28) f(29) \
f(30) f(31)

#define PUSH(n) STORE concat(x, n), (n * XLEN)(sp);
#define POP(n)  LOAD  concat(x, n), (n * XLEN)(sp);

#define CONTEXT_SIZE  ((32 + 3 + 1) * XLEN)
#define OFFSET_SP     ( 2 * XLEN)
#define OFFSET_CAUSE  (32 * XLEN)
#define OFFSET_STATUS (33 * XLEN)
#define OFFSET_EPC    (34 * XLEN)

.align 3
.globl __am_asm_trap
__am_asm_trap:

  addi sp, sp, -CONTEXT_SIZE

  MAP(REGS, PUSH)
  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc
  STORE t0, OFFSET_CAUSE(sp)
  STORE t1, OFFSET_STATUS(sp)
  STORE t2, OFFSET_EPC(sp)
  # 多道程序中，各进程空间是隔离的，每个进程的内存状态机分析范围缩小到每个进程空间。
  # 到此，包括正确的SP在内的所有状态得到了保存，内存中各进程空间是个隔离的，根据这些状态完全可以还原进程的运行。

  # set mstatus.MPRV to pass difftest
  li a0, (1 << 17)
  or t1, t1, a0
  csrw mstatus, t1

  # 一个子程序从开始到结束，出栈操作次数一般等于入栈操作次数，__am_irq_handle调用完成后，sp指针依旧是上一个进程的sp指针。
  mv a0, sp
  jal __am_irq_handle

  # 异常服务完成后，仅需根据各进程的SP，就能选择还原某个进程。
  mv a0, sp
  jal schedule
  mv sp, a0

  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)
  csrw mstatus, t1
  csrw mepc, t2
  MAP(REGS, POP)

  addi sp, sp, CONTEXT_SIZE

  mret
