#include <memory.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
	char* pp = pf;
	pf += nr_page;
	return pp;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
	void *pp = new_page(n * PGSIZE);
	return pp;
}
#endif

void free_page(void *p) {
	panic("free_page not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  return 0;
}

//  ��һ����ǽ�TRM�ṩ�Ķ�����ʼ��ַ��Ϊ��������ҳ���׵�ַ, �����Ժ�, �����Ϳ���ͨ��new_page()������������е�����ҳ�ˡ�
void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
