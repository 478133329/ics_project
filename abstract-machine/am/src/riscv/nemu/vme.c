#include <am.h>
#include <nemu.h>
#include <klib.h>

// �ں������ַ�ռ�
static AddrSpace kas = {};

static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

// riscv64
void map(AddrSpace *as, void *va, void *pa, int prot) {
  int pd_idx = ((uintptr_t)va >> 30) & 0x1ff;
  int pt1_idx = ((uintptr_t)va >> 21) & 0x1ff;
  int pt2_idx = ((uintptr_t)va >> 12) & 0x1ff;

  page_entry *page_dir_entry = &((page_entry*)as.ptr)[pd_idx];
  if (page_dir_entry->V == 0) {
    page_entry *page_tbl1 = pgalloc_usr(PGSIZE);
    page_dir_ertry->PPN = (uintptr_t)page_tbl1 >> 12;
    page_dir_entry->V = 1;
  }

  uintptr_t page_tbl1_addr = page_dir_ertry.PPN << 12;
  page_entry *page_tbl1_entry = &((page_entry*)page_tbl1_addr)[pt1_idx];
  if (page_tbl1_entry->V == 0) {
    page_entry *page_tbl2 = pgalloc_usr(PGSIZE);
    page_tbl1_entry->PPN = (uintptr_t)page_tbl2 >> 12;
    page_tbl1_entry->V = 1;
  }

  uintptr_t page_tbl2_addr = page_dir_ertry.PPN << 12;
  page_entry *page_tbl2_entry = &((page_entry*)page_tbl2_addr)[pt2_idx];
  if (page_tbl2_entry->V == 0) {
    page_tbl1_entry->PPN = (uintptr_t)pa >> 12;
    page_tbl1_entry->V = 1;
  }
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
    Context *cp = (Context*)((uint8_t*)kstack.end - sizeof(Context) + 1);

    cp->mstatus = 0xa00001800;

    cp->mepc = (uintptr_t)entry;

    // ����һ������ʱ��ֻҪ����һ������pc��Ȼ���ִ�н���_start����_start����Ҫ�Ĺ���֮һ���ǳ�ʼ��C������Ҳ���ǳ�ʼ��sp��
    // cp->sp = (uintptr_t)cp;
    
    return cp;
}
