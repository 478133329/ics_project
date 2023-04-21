#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include "ftracer.h"

// static FUNC_INFO func_infos[1024];

void init_ftracer(const char* elf_file) {
	FILE* elf = fopen(elf_file, "rb");
	assert(elf != NULL);

	// Elf32_Ehdr elf_header;

}
