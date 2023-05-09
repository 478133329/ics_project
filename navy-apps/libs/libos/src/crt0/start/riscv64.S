// 用户程序的入口地址，不要和由AM提供的操作系统的入口地址混淆。
// 在操作系统不支持内存管理的其概况下，我们要注意用户程序的地址空间不要和操作系统的重叠。
// 地址的设置在navy-apps/scripts/riscv64.mk的LDFLAGS中。

.globl  _start
_start:
  move sp, a0
  move s0, zero

  jal call_main
