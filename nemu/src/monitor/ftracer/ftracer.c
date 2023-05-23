#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include "ftracer.h"

// static FUNC_INFO call_stack[128];
// static int p = 0;

void init_ftracer(const char* elf_file) {
	// 根据路径，读取进程外的elf文件
	FILE* elf = fopen(elf_file, "rb");
	assert(elf != NULL);

	// 解析elf文件，搜集函数信息
	// Elf32_Ehdr elf_header;

}

void is_call_or_ret(vaddr_t dnpc, vaddr_t rd) {

}

