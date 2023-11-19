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

#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>

/*
memory
enum { MMU_DIRECT, MMU_TRANSLATE, MMU_FAIL };
enum { MEM_TYPE_IFETCH, MEM_TYPE_READ, MEM_TYPE_WRITE };
enum { MEM_RET_OK, MEM_RET_FAIL, MEM_RET_CROSS_PAGE };
*/

/*
satp mode
0: bare
8: sv39
9: sv48  ignore
*/

// riscv64-sv39
typedef union{
	paddr_t val;
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

static void check_valid(page_entry pe) {
	assert(pe.V);
}

int isa_mmu_check(vaddr_t vaddr, int len, int type) {
	int mode = cpu.csr[4] >> 60;
	if (mode == 8) {
		return MMU_TRANSLATE;
	}
	return MMU_DIRECT;
}

/*
page_entry page_dir_entry = ((page_entry*)page_dir_addr)[pd_idx];
error: cast to pointer from integer of different size [-Werror=int-to-pointer-case]]
¸ÄÎª page_entry page_dir_entry = ((page_entry*)(uintptr_t)page_dir_addr)[pd_idx];
*/
paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
	int pd_idx = (vaddr >> 30) & 0x1ff;
	int pt1_idx = (vaddr >> 21) & 0x1ff;
	int pt2_idx = (vaddr >> 12) & 0x1ff;
	int offset = vaddr & 0xfff;
	paddr_t page_dir_addr = (cpu.csr[4] & 0xfffffffffff) << 12;
	page_entry page_dir_entry = ((page_entry*)(uintptr_t)page_dir_addr)[pd_idx];
	check_valid(page_dir_entry);
	paddr_t page_tbl1_addr = page_dir_entry.PPN << 12;
	page_entry page_tbl1_entry = ((page_entry*)(uintptr_t)page_tbl1_addr)[pt1_idx];
	check_valid(page_tbl1_entry);
	paddr_t page_tbl2_addr = page_tbl1_entry.PPN << 12;
	page_entry page_tbl2_entry = ((page_entry*)(uintptr_t)page_tbl2_addr)[pt2_idx];
	check_valid(page_tbl2_entry);
	paddr_t paddr = (page_tbl2_entry.PPN << 12) | offset;
	return paddr;
}
