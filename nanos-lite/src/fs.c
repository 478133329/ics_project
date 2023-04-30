#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t current_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
	panic("should not reach here");
	return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
	panic("should not reach here");
	return 0;
}

size_t putch_write(const void* buf, size_t offset, size_t len) {
	int i = 0;
	for (i = 0; i < len; i++) {
		putch(((char*)buf)[offset + i]);
	}
	return i;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
	// 参数列表初始数组 int a[] = {1, 2, 3}。
	// [指定下标] = 4 可使用这种方式指定下标。
	[FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
	[FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, putch_write},
	[FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, putch_write},
	{"/bin/hello", 36816, 0},
	{"/bin/dummy", 33064, 36816},

	#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char* pathname, int flags, int mode) {
	// 获取数组中元素的个数，可以使用 sizeof(a) / sizeof(a[0]) 的形式来计算。
	int file_num = sizeof(file_table) / sizeof(file_table[0]);
	for (int i = 0; i < file_num; i++) {
		if (strcmp(pathname, file_table[i].name) == 0) {
			return i;
		}
	}
	assert(0);
}

size_t ramdisk_read(void* buf, size_t offset, size_t len);

size_t ramdisk_write(const void* buf, size_t offset, size_t len);

size_t fs_read(int fd, void* buf, size_t len) {
	size_t ret = 0;
	if (file_table[fd].read != NULL) {
		size_t offset = file_table[fd].disk_offset;
		file_table[fd].read(buf, offset, len);
	}
	else {
		if (file_table[fd].current_offset + len >= file_table[fd].size) assert(0);
		size_t offset = file_table[fd].disk_offset + file_table[fd].current_offset;
		ret = ramdisk_read(buf, offset, len);
		file_table[fd].current_offset += len;
	}
	return ret;
}

// fs_write函数是文件系统具体实现（例如Ext2、NTFS等）提供的接口函数。
size_t fs_write(int fd, const void* buf, size_t len) {
	size_t ret = 0;
	if (file_table[fd].write != NULL) {
		size_t offset = file_table[fd].disk_offset;
		file_table[fd].write(buf, offset, len);
	}
	else {
		if (file_table[fd].current_offset + len >= file_table[fd].size) assert(0);
		size_t offset = file_table[fd].disk_offset + file_table[fd].current_offset;
		ret = ramdisk_write(buf, offset, len);
		file_table[fd].current_offset += len;
	}
	return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
	int off = offset;
	switch (whence) {
	case SEEK_SET:
		if (off < 0 || off >= file_table[fd].size) assert(0);
		file_table[fd].current_offset = off;
		break;
	case SEEK_CUR:
		if (file_table[fd].current_offset + off < 0 || file_table[fd].current_offset + off >= file_table[fd].size) assert(0);
		file_table[fd].current_offset += off;
		break;
	case SEEK_END:
		if (off > 0 || file_table[fd].size + off <= 0) assert(0);
		file_table[fd].current_offset += off;
	}
	return file_table[fd].current_offset;
}

int fs_close(int fd) {
	return 0;
}