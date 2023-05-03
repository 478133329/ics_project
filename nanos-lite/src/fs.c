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


// �ֽ�����û��"λ��"�ĸ���, ���serial_write()�е�offset�������Ժ���
size_t serial_write(const void* buf, size_t offset, size_t len);
size_t events_read(void* buf, size_t offset, size_t len);
size_t dispinfo_read(void* buf, size_t offset, size_t len);
size_t fb_write(const void* buf, size_t offset, size_t len);
size_t ramdisk_read(void* buf, size_t offset, size_t len);
size_t ramdisk_write(const void* buf, size_t offset, size_t len);

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
	// �����б��ʼ���� int a[] = {1, 2, 3}��
	// [ָ���±�] = 4 ��ʹ�����ַ�ʽָ���±ꡣ
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
	// ��ȡ������Ԫ�صĸ���������ʹ�� sizeof(a) / sizeof(a[0]) ����ʽ�����㡣
	int file_num = sizeof(file_table) / sizeof(file_table[0]);
	for (int i = 0; i < file_num; i++) {
		if (strcmp(pathname, file_table[i].name) == 0) {
			return i;
		}
	}
	assert(0);
}

/*
�ļ���С�̶���read��writeʱӦע��ƫ������Ҫ����SEEK_END��
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

// fs_write�������ļ�ϵͳ����ʵ�֣�����Ext2��NTFS�ȣ��ṩ�Ľӿں�����
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
		// SEEK_END����ָ���ļ����һ���ֽڣ��������һ���ֽں�һ��λ�ã����������ļ����ݣ����Է�ӳ�ļ���С��
		file_table[fd].current_offset = file_table[fd].size + off;
		break;
	}
	return file_table[fd].current_offset;
}

int fs_close(int fd) {
	return 0;
}