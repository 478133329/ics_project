#include <proc.h>
#include <elf.h>

#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))

size_t ramdisk_read(void* buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
	// error: Elf_Ehdr *ehdr = NULL
	Elf_Ehdr ehdr = {};
	Elf_Phdr phdr = {};
	// ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
	int fd = fs_open(filename, 0, 0);
	fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
	Elf64_Off phdr_off = ehdr.e_phoff;
	uint16_t phdr_entsize = ehdr.e_phentsize;
	uint16_t phdr_phnum = ehdr.e_phnum;
	for (int i = 0; i < phdr_phnum; i++) {
		fs_lseek(fd, phdr_off + i * phdr_entsize, SEEK_SET);
		// ramdisk_read(&phdr, phdr_off + i * phdr_entsize, sizeof(Elf_Phdr));
		fs_read(fd, &phdr, sizeof(Elf_Phdr));
		if (phdr.p_type == PT_LOAD) {
			// memcpy((char*)phdr.p_vaddr, &ramdisk_start + phdr.p_offset, phdr.p_filesz);
			fs_lseek(fd, phdr.p_offset, SEEK_SET);
			fs_read(fd, (char*)phdr.p_vaddr, phdr.p_filesz);
			memset((char*)phdr.p_vaddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
		}
	}
	return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char* filename) {
	uintptr_t entry = loader(pcb, filename);
	Log("Jump to entry = %p", entry);
	// 借助函数指针 void (*) ()，完成一个简单的跳转指令。
	((void (*) ())entry) ();
}

void context_kload(PCB* pcb, void (*entry)(void*), void* arg) {
	Area stack;
	stack.start = &pcb->stack;
	stack.end = &pcb->stack[STACK_SIZE - 1];
	Context* cp = kcontext(stack, entry, arg);
	// 指针转换为整形，使用intptr_t类型，因为不同平台指针类型可能为32/64位。
	pcb->cp = cp;
}

void context_uload(PCB* pcb, const char* filename, char* const argv[], char* const envp[]) {
	uintptr_t entry = loader(pcb, filename);

	Area stack;
	stack.start = &pcb->stack;
	stack.end = &pcb->stack[STACK_SIZE - 1];

	Context* cp = ucontext(NULL, stack, (void*)entry);

	int argc = 0;
	int envc = 0;
	if (argv) {
		for (; argv[argc]; ++argc) {}
	}
	if (envp) {
		for (; envp[envc]; ++envc) {}
	}

	int str_size = 0;
	for (int i = 0; i < argc; i++) {
		str_size += strlen(argv[i]);
	}
	for (int i = 0; i < envc; i++) {
		str_size += strlen(envp[i]);
	}

	// 将GPRx设置为argc所在的地址
	cp->GPRx = (uintptr_t)((uint8_t*)heap.end - (argc + envc + 1 + 2) * sizeof(uintptr_t) - str_size);  // heap.end = 0x88000000 越界

	// ucos中在OSTaskInit时，也是通过参数寄存器传递void* arg。

	// 先确定args在栈中的内存结构。
	int* argc_ptr = (int*)cp->GPRx;

	uintptr_t* argv_ptr = (uintptr_t*)(argc_ptr + 1);

	uintptr_t* envp_ptr = argv_ptr + argc + 1;

	char* str_ptr = (char*)(envp_ptr + envc + 1);
	
	// 向argc、argv、envp、str区域中填值。
	*argc_ptr = argc;
	
	for (int i = 0; i < argc; i++) {
		int n = strlen(argv[i]);
		memcpy(str_ptr, argv[i], n);
		((char**)argv_ptr)[i] = (char*)str_ptr;
		str_ptr += n;
	}
	
	for (int i = 0; i < envc; i++) {
		int n = strlen(envp[i]);
		memcpy(str_ptr, envp[i], n);
		((char**)envp_ptr)[i] = (char*)str_ptr;
		str_ptr += n;
	}

	pcb->cp = cp;
}
