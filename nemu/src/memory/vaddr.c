/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/
#include <stdio.h>
#include <isa.h>
#include <memory/paddr.h>

// int isa_mmu_check(vaddr_t vaddr, int len, int type);
// paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type);
// enum { MEM_TYPE_IFETCH, MEM_TYPE_READ, MEM_TYPE_WRITE };
// enum { MMU_DIRECT, MMU_TRANSLATE, MMU_FAIL };
word_t vaddr_ifetch(vaddr_t addr, int len) {
	paddr_t paddr = addr;
	int res = isa_mmu_check(addr, len, MEM_TYPE_IFETCH);
	if (res == MMU_TRANSLATE) {
		paddr = isa_mmu_translate(addr, len, MEM_TYPE_IFETCH);
	}
	return paddr_read(paddr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
	paddr_t paddr = addr;
	int res = isa_mmu_check(addr, len, MEM_TYPE_READ);
	if (res == MMU_TRANSLATE) {
		paddr = isa_mmu_translate(addr, len, MEM_TYPE_READ);
	}
	return paddr_read(paddr, len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
	paddr_t paddr = addr;
	int res = isa_mmu_check(addr, len, MEM_TYPE_WRITE);
	if (res == MMU_TRANSLATE) {
		paddr = isa_mmu_translate(addr, len, MEM_TYPE_WRITE);
	}
	paddr_write(paddr, len, data);
}
