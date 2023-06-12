#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define keyname(k) #k,

static const char* keyname[] = {
  "NONE",
  _KEYS(keyname)
};

struct __event_element {
	uint8_t type;
	uint8_t sym;
	struct __event_element* next;
};
typedef struct __event_element event_element;

static event_element event_queue = { .type = 0, .sym = 0, .next = NULL };
static event_element* end = &event_queue;

static void append(uint8_t type, uint8_t sym) {
	event_element* new_element = malloc(sizeof(event_element));
	new_element->type = type;
	new_element->sym = sym;
	new_element->next = NULL;
	end->next = new_element;
	end = new_element;
}

static int pop(uint8_t* type, uint8_t* sym) {
	if (event_queue.next == NULL) {
		return 0;
	}
	else {
		event_element* buf = event_queue.next;
		*type = buf->type;
		*sym = buf->sym;
		event_queue.next = buf->next;
		if (buf == end) {
			end = &event_queue;
		}
		free(buf);
	}
	return 1;
}

static uint8_t key_state[sizeof(keyname) / sizeof(keyname[0])] = { 0 };

int SDL_PushEvent(SDL_Event* ev) {
	assert(0);
	return 0;
}

static char key_buf[64], * key_action, * key_key;
//To Be Fast

int SDL_PollEvent(SDL_Event* ev) {
	// 键盘事件
	char event_buf[64];
	int ret = NDL_PollEvent(event_buf, sizeof(event_buf));
	if (ret == 0) {
		return ret;
	}
	char k1[10], k2[10];
	sscanf(event_buf, "%s %s", k1, k2);
	if (strcmp(k1, "kd") == 0) {
		ev->type = SDL_KEYDOWN;
	}
	else if (strcmp(k1, "ku") == 0) {
		ev->type = SDL_KEYUP;
	}
	else {}
	for (int i = 0; i < (sizeof(keyname) / sizeof(char*)); i++) {
		if (strcmp(k2, keyname[i]) == 0) {
			ev->key.keysym.sym = i;
		}
	}
	return ret;
}

int SDL_WaitEvent(SDL_Event* event) {
	// 键盘事件
	char event_buf[64];
	int ret = NDL_PollEvent(event_buf, sizeof(event_buf));
	while (!ret) {
		ret = NDL_PollEvent(event_buf, sizeof(event_buf));
	}
	char k1[10], k2[10];
	sscanf(event_buf, "%s %s", k1, k2);
	if (strcmp(k1, "kd") == 0) {
		event->type = SDL_KEYDOWN;
	}
	else if (strcmp(k1, "ku") == 0) {
		event->type = SDL_KEYUP;
	}
	else {}
	for (int i = 0; i < (sizeof(keyname) / sizeof(char*)); i++) {
		if (strcmp(k2, keyname[i]) == 0) {
			event->key.keysym.sym = i;
		}
	}
	return ret;
}

int SDL_PeepEvents(SDL_Event* ev, int numevents, int action, uint32_t mask) {
	assert(0);
	return 0;
}

uint8_t* SDL_GetKeyState(int* numkeys) {
	SDL_Event ev;

	if (numkeys)
		*numkeys = sizeof(key_state) / sizeof(key_state[0]);
	//SDL_PumpEvents();
	return key_state;
}