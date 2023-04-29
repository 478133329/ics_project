#include <common.h>
#include <syscall.h>

// ��Linux�ں��У�ϵͳ���ô�����������ͨ����ѭ"sys_"ǰ׺������Լ������sys_write��sys_read�ȡ�

void sys_write(Context* c) {
    // fd, buf, count
    int fd = c->GPR2;
    // ָ��ת��Ϊ����ͨ����������棬������ת��Ϊָ������ܻᵼ�±�������������ʱ����
    // ָ��ת��Ϊ������ͨ��ת��Ϊuintptr_t����Ϊ��Բ�ͬƽ̨�������������롣
    // ������������ָ�뵽������ת������Ӧ��ʹ����ʽ������ת����
    void* buf = (void*)c->GPR3;
    size_t count = c->GPR4;
    if (fd == 1 || fd == 2) {
        int i = 0;
        for (i = 0; i < count; i++) {
            putch(((char*)buf)[i]);
        }
        c->GPRx = i;
    }
}

void sys_brk(Context* c) {
    c->GPRx = 0;
}

void do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    Log("syscall %d - ", a[0]);
    switch (a[0]) {
    case SYS_exit: halt(0);
    case SYS_yield: yield(); break;
    case SYS_write: sys_write(c); break;
    case SYS_brk: sys_brk(c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
    }
    Log("syscall %d over\n ", a[0]);
}
