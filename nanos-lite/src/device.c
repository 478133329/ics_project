#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
	#define MULTIPROGRAM_YIELD() yield()
#else
	#define MULTIPROGRAM_YIELD()
#endif

// ##          连接符            #define CONCAT(x, y) x ## y         ->  CONCAT(for, bar) = foobar
// #           转字符串常量符    #define STR(x) #x,                  ->  STR(foobar) = "foobar",
// (_) _() _() 批量调用另一个宏  #define BATCH(_) _(hello) _(world)  ->  BATCH(STR) =  "hello", "world",

// int a[] = {1, 2, 3, };  ->  int a[] = {1, 2, 3, 0};

#define NAME(key) \
	[AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
	[AM_KEY_NONE] = "NONE",
	AM_KEYS(NAME)
};

/*
	#define AM_KEYS(_) \
		_(ESCAPE) _(F1) _(F2) _(F3) _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) \
		_(GRAVE) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) _(MINUS) _(EQUALS) _(BACKSPACE) \
		_(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) \
		_(CAPSLOCK) _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) _(SEMICOLON) _(APOSTROPHE) _(RETURN) \
		_(LSHIFT) _(Z) _(X) _(C) _(V) _(B) _(N) _(M) _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
		_(LCTRL) _(APPLICATION) _(LALT) _(SPACE) _(RALT) _(RCTRL) \
		_(UP) _(DOWN) _(LEFT) _(RIGHT) _(INSERT) _(DELETE) _(HOME) _(END) _(PAGEUP) _(PAGEDOWN)

	#define AM_KEY_NAMES(key) AM_KEY_##key,

	enum {
		AM_KEY_NONE = 0,
		AM_KEYS(AM_KEY_NAMES)
	};
*/

size_t serial_write(const void *buf, size_t offset, size_t len) {
	int i = 0;
	for (i = 0; i < len; i++) {
		putch(((char*)buf)[i]);
	}
	return i;
}

size_t events_read(void *buf, size_t offset, size_t len) {
	// AM_DEVREG( 9, INPUT_KEYBRD,   RD, bool keydown; int keycode);
	// 字符设备不用关注offset。
	AM_INPUT_KEYBRD_T input = io_read(AM_INPUT_KEYBRD);
	if (input.keycode != 0) {
		// char key[] = keyname[input.keycode];  error: 不能在声明数组时使用另一个数组的元素来初始化它。
		char key[20];
		char result[20];
		strncpy(key, keyname[input.keycode], 20);
		if (input.keydown == true) {
			sprintf(result, "kd %s\n", key);
		}
		else {
			sprintf(result, "ku %s\n", key);
		}
		printf("result %s\n", result);
		strncpy(buf, result, len);
		int ret = (strlen(result) < len) ? strlen(result) : len;
		return ret;
	}
	return 0;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
	return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
	return 0;
}

void init_device() {
	Log("Initializing devices...");
	ioe_init();
}
