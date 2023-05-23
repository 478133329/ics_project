#include <NDL.h>
#include <SDL.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
	// ¼üÅÌÊÂ¼ş
	char* event_buf;
	int ret = NDL_PollEvent(event_buf, 20);
	if (ret == 0) {
		return ret;
	}
	char k1[10], k2[10];
	sscanf(event_buf, "%s %s", k1, k2);
	switch (k1) {
	case "kd": 	ev->type = SDL_KEYDOWN; ev->key.type = SDL_KEYDOWN; ev->key.state = SDL_PRESSED; break;
	case "ku": 	ev->type = SDL_KEYUP; ev->key.type = SDL_KEYUP; ev->key.state = SDL_RELEASED; break;
	default: break;
	}
	ev->key.keysym.scancode = 0;
	switch (k2) {
	case "0": 	ev->key.keysym.sym = SDLK_0; break;
	case "1": 	ev->key.keysym.sym = SDLK_1; break;
	case "2": 	ev->key.keysym.sym = SDLK_2; break;
	case "3": 	ev->key.keysym.sym = SDLK_3; break;
	case "4": 	ev->key.keysym.sym = SDLK_4; break;
	case "5": 	ev->key.keysym.sym = SDLK_5; break;
	case "6": 	ev->key.keysym.sym = SDLK_6; break;
	case "7": 	ev->key.keysym.sym = SDLK_7; break;
	case "8": 	ev->key.keysym.sym = SDLK_8; break;
	case "9": 	ev->key.keysym.sym = SDLK_9; break;
	case "J": 	ev->key.keysym.sym = SDLK_J; break;
	case "DOWN": 	ev->key.keysym.sym = SDLK_DOWN; break;
	case "K": 	ev->key.keysym.sym = SDLK_K; break;
	case "UP": 	ev->key.keysym.sym = SDLK_UP; break;
	case "G": 	ev->key.keysym.sym = SDLK_G; break;
	default: break;
	}
	return ret;
}

int SDL_WaitEvent(SDL_Event *event) {
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
