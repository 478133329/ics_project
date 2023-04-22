#include <am.h>
#include <nemu.h>

extern char _heap_start;
int main(const char *args);

Area heap = RANGE(&_heap_start, PMEM_END);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

/*
使用设备的需求->设备的访问->设备控制寄存器访问
CPU只认识已经完成了独立编址的寄存器堆和地址线提供的寻址空间
使用内存统一编址进行设备寻址
各平台设备地址不同，且有的设备地址可编程，因此设备编址由AM进行一次抽象很必要，向上提供统一接口
对于总线的抽象：设备地址译码由mmio模拟完成，忽略时序控制
对于设备的执行：
	立即接收数据，只有一个一字节数据寄存器用于接收数据
	立即计算完成，printf模拟串口输出功能
	立即返回结果，执行完成，清空数据寄存器
*/

void putch(char ch) {
	// static inline void outb(uintptr_t addr, uint8_t  data) { *(volatile uint8_t  *)addr = data; }
  outb(SERIAL_PORT, ch);
}

void halt(int code) {
	;
  nemu_trap(code);

  // should not reach here
  while (1);
}

void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
