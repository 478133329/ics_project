// �û��������ڵ�ַ����Ҫ����AM�ṩ�Ĳ���ϵͳ����ڵ�ַ������
// �ڲ���ϵͳ��֧���ڴ��������ſ��£�����Ҫע���û�����ĵ�ַ�ռ䲻Ҫ�Ͳ���ϵͳ���ص���
// ��ַ��������navy-apps/scripts/riscv64.mk��LDFLAGS�С�

.globl  _start
_start:
  move s0, zero
  jal call_main