#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include "ftracer.h"

// static FUNC_INFO call_stack[128];
// static int p = 0;

void init_ftracer(const char* elf_file) {
	// ����·������ȡ�������elf�ļ�
	FILE* elf = fopen(elf_file, "rb");
	assert(elf != NULL);

	// ����elf�ļ����Ѽ�������Ϣ
	// Elf32_Ehdr elf_header;

}

void is_call_or_ret(vaddr_t dnpc, vaddr_t rd) {

}

