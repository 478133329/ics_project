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
ʹ���豸������->�豸�ķ���->�豸���ƼĴ�������
CPUֻ��ʶ�Ѿ�����˶�����ַ�ļĴ����Ѻ͵�ַ���ṩ��Ѱַ�ռ�
ʹ���ڴ�ͳһ��ַ�����豸Ѱַ
��ƽ̨�豸��ַ��ͬ�����е��豸��ַ�ɱ�̣�����豸��ַ��AM����һ�γ���ܱ�Ҫ�������ṩͳһ�ӿ�
�������ߵĳ����豸��ַ������mmioģ����ɣ�����ʱ�����
�����豸��ִ�У�
	�����������ݣ�ֻ��һ��һ�ֽ����ݼĴ������ڽ�������
	����������ɣ�printfģ�⴮���������
	�������ؽ����ִ����ɣ�������ݼĴ���
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
