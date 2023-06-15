#include <NDL.h>
#include <SDL.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

static uint8_t key_state[sizeof(keyname) / sizeof(keyname[0])] = { 0 };

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

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

	switch (ev->type) {
	case SDL_KEYDOWN:
		key_state[ev->key.keysym.sym] = 1;
		break;

	case SDL_KEYUP:
		key_state[ev->key.keysym.sym] = 0;
		break;
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

	switch (event->type) {
	case SDL_KEYDOWN:
		key_state[event->key.keysym.sym] = 1;
		break;

	case SDL_KEYUP:
		key_state[event->key.keysym.sym] = 0;
		break;
	}

	return ret;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
	SDL_Event ev;
	if (numkeys)
		*numkeys = sizeof(key_state) / sizeof(key_state[0]);
	//SDL_PumpEvents();
	return key_state;
}
