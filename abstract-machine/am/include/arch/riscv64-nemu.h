#ifndef ARCH_H__
#define ARCH_H__

struct Context {
  // TODO: fix the order of these members to match trap.S
  uintptr_t gpr[32], mcause, mstatus, mepc;
  void *pdir;
};

#define GPR1 gpr[17] // a7
#define GPR2 gpr[10] // a0
#define GPR3 gpr[11] // a1
#define GPR4 gpr[12] // a2
#define GPRx gpr[10] // a0

#define ra   gpr[1]  // ra
#define sp   gpr[2]  // sp

// riscv64-sv39
typedef union{
	uintptr_t val;
	struct {
		uint64_t V : 1;
		uint64_t R : 1;
		uint64_t W : 1;
		uint64_t X : 1;
		uint64_t U : 1;
		uint64_t G : 1;
		uint64_t A : 1;
		uint64_t D : 1;
		uint64_t RSW : 2;
		uint64_t PPN : 44;
		uint64_t Reserved : 10;
	} ;
} page_entry;

#endif
