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
	// assert(0);
	/*
	这是一段Newlibc中的源码
	int
	_open_r (struct _reent *ptr,
		 const char *file,
		int flags,
		int mode)
	{
		int ret;

		errno = 0;
		if ((ret = _open (file, flags, mode)) == -1 && errno != 0)
			ptr->_errno = errno;
		return ret;
	}
	这里的_open由libos提供
	可以看到，libc和libos应严格遵循一些约定，比如当open失败的时候应当返回-1，而不是搞一些assert等操作
	
	*/

	/*
	关于nslider在最后一页向下翻页导致报错的原因，最终定位在此，fs_open返回了-1，这是为什么？
	一共两张图片 slider0和slider1
	在slider1向下翻页的时候应当继续显示slider1，但实际上是要去显示slider2，找不到该文件，应用层assert。
	最终问题在于nslider的main函数中没有重新定义正确的N来表示幻灯片的个数。
	*/
	return -1;
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
	Finfo* info = &file_table[fd];

	switch (whence) {
	case SEEK_CUR:
		assert(info->current_offset + offset <= info->size);
		info->current_offset += offset;
		break;

	case SEEK_SET:
		assert(offset <= info->size);
		info->current_offset = offset;
		break;

	case SEEK_END:
		assert(offset <= info->size);
		info->current_offset = info->size + offset;
		break;

	default:
		assert(0);
	}

	return info->current_offset;
}

int fs_close(int fd) {
	file_table[fd].current_offset = 0;
	return 0;
}