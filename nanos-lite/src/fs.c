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
	// assert(0);
	/*
	����һ��Newlibc�е�Դ��
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
	�����_open��libos�ṩ
	���Կ�����libc��libosӦ�ϸ���ѭһЩԼ�������統openʧ�ܵ�ʱ��Ӧ������-1�������Ǹ�һЩassert�Ȳ���
	
	*/

	/*
	����nslider�����һҳ���·�ҳ���±����ԭ�����ն�λ�ڴˣ�fs_open������-1������Ϊʲô��
	һ������ͼƬ slider0��slider1
	��slider1���·�ҳ��ʱ��Ӧ��������ʾslider1����ʵ������Ҫȥ��ʾslider2���Ҳ������ļ���Ӧ�ò�assert��
	������������nslider��main������û�����¶�����ȷ��N����ʾ�õ�Ƭ�ĸ�����
	*/
	return -1;
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