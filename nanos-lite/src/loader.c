#include <proc.h>
#include <elf.h>

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


#include <stdio.h>

static uintptr_t loader(PCB *pcb, const char *filename) {
	// error: Elf_Ehdr *ehdr = NULL
	Elf_Ehdr ehdr = {};
	Elf_Phdr phdr = {};
	ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
	Elf64_Off phdr_off = ehdr.e_phoff;
	uint16_t phdr_entsize = ehdr.e_phentsize;
	uint16_t phdr_phnum = ehdr.e_phnum;
	for (int i = 0; i < phdr_phnum; i++) {
		ramdisk_read(&phdr, phdr_off + i * phdr_entsize, sizeof(Elf_Phdr));
		if (phdr.p_type == PT_LOAD) {
			memcpy((char*)phdr.p_vaddr, &ramdisk_start + phdr.p_offset, phdr.p_filesz);
			memset((char*)phdr.p_vaddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
		}
	}
	return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
	uintptr_t entry = loader(pcb, filename);
	Log("Jump to entry = %p", entry);
	// 借助函数指针 void (*) ()，完成一个简单的跳转指令。
	((void (*) ())entry) ();
}

