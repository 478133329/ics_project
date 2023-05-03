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

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENT, FD_DISPINFO, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
	panic("should not reach here");
	return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
	panic("should not reach here");
	return 0;
}


// 字节序列没有"位置"的概念, 因此serial_write()中的offset参数可以忽略
size_t serial_write(const void* buf, size_t offset, size_t len);
size_t events_read(void* buf, size_t offset, size_t len);
size_t dispinfo_read(void* buf, size_t offset, size_t len);
size_t fb_write(const void* buf, size_t offset, size_t len);
size_t ramdisk_read(void* buf, size_t offset, size_t len);
size_t ramdisk_write(const void* buf, size_t offset, size_t len);

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
	// 参数列表初始数组 int a[] = {1, 2, 3}。
	// [指定下标] = 4 可使用这种方式指定下标。
	[FD_STDIN]    = {"stdin"         , 0, 0, 0, invalid_read , invalid_write},
	[FD_STDOUT]   = {"stdout"        , 0, 0, 0, invalid_read , serial_write },
	[FD_STDERR]   = {"stderr"        , 0, 0, 0, invalid_read , serial_write },
	[FD_EVENT]    = {"/dev/event"    , 0, 0, 0, events_read  , invalid_write},
	[FD_DISPINFO] = {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
	[FD_FB]       = {"/dev/fb"       , 0, 0, 0, invalid_read , fb_write     },

	#include "files.h"
};

void init_fs() {
	// TODO: initialize the size of /dev/fb
	AM_GPU_CONFIG_T config = io_read(AM_GPU_CONFIG);
	file_table[FD_FB].size = config.width * config.height * sizeof(uint32_t);
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

/*
文件大小固定，read、write时应注意偏移量不要超过SEEK_END。
*/

size_t fs_read(int fd, void* buf, size_t len) {
	size_t ret = 0;
	if (file_table[fd].read != NULL) {
		size_t offset = /*file_table[fd].disk_offset*/ + file_table[fd].current_offset;
		ret = file_table[fd].read(buf, offset, len);
	}
	else {
		size_t offset = file_table[fd].disk_offset + file_table[fd].current_offset;
		ret = ramdisk_read(buf, offset, len);
		file_table[fd].current_offset = (file_table[fd].current_offset + len < file_table[fd].size) ? file_table[fd].current_offset + len : file_table[fd].size;
	}
	return ret;
}

// fs_write函数是文件系统具体实现（例如Ext2、NTFS等）提供的接口函数。
size_t fs_write(int fd, const void* buf, size_t len) {
	size_t ret = 0;
	if (file_table[fd].write != NULL) {
		size_t offset = /*file_table[fd].disk_offset*/ + file_table[fd].current_offset;
		ret = file_table[fd].write(buf, offset, len);
	}
	else {
		size_t offset = file_table[fd].disk_offset + file_table[fd].current_offset;
		ret = ramdisk_write(buf, offset, len);
		file_table[fd].current_offset = (file_table[fd].current_offset + len < file_table[fd].size) ? file_table[fd].current_offset + len : file_table[fd].size;
	}
	return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
	int off = offset;
	switch (whence) {
	case SEEK_SET:
		if (off < 0 || off > file_table[fd].size) assert(0);
		file_table[fd].current_offset = off;
		break;
	case SEEK_CUR:
		if (file_table[fd].current_offset + off < 0 || file_table[fd].current_offset + off > file_table[fd].size) { printf("current_offset: %d, len: %d, size: %d\n", (int)file_table[fd].current_offset, (int)off, (int)file_table[fd].size);  assert(0); }
		file_table[fd].current_offset += off;
		break;
	case SEEK_END:
		if (off > 0 || file_table[fd].size + off < 0) assert(0);
		// error: file_table[fd].current_offset = file_table[fd].size - 1 + off;
		// SEEK_END并不指向文件最后一个字节，而是最后一个字节后一个位置，它不包含文件数据，可以反映文件大小。
		file_table[fd].current_offset = file_table[fd].size + off;
		break;
	}
	return file_table[fd].current_offset;
}

int fs_close(int fd) {
	return 0;
}