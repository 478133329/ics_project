#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <fcntl.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;

uint32_t NDL_GetTicks() {
    struct timeval tv = {};
    gettimeofday(&tv, NULL);
    return (uint32_t)tv.tv_usec / 1000;
}

/*
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
*/

int NDL_PollEvent(char *buf, int len) {
    evtdev = open("/dev/event", 0);
    int ret = read(evtdev, buf, len);
    return ret;
}

#define GPU_CONFIG_SIZE 50

static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;

void NDL_OpenCanvas(int *w, int *h) {
    if (*w == 0 && *h == 0) {
        canvas_w = screen_w;
        canvas_h = screen_h;
    }
    else {
        if (*w > screen_w || *h > screen_h) {
            assert(0);
        }
        canvas_w = *w;
        canvas_h = *w;
    }

    if (getenv("NWM_APP")) {
        int fbctl = 4;
        fbdev = 5;
        screen_w = *w; screen_h = *h;
        char buf[64];
        int len = sprintf(buf, "%d %d", screen_w, screen_h);
        // let NWM resize the window and create the frame buffer
        write(fbctl, buf, len);
        while (1) {
            // 3 = evtdev
            int nread = read(3, buf, sizeof(buf) - 1);
            if (nread <= 0) continue;
            buf[nread] = '\0';
            if (strcmp(buf, "mmap ok") == 0) break;
        }
        close(fbctl);
    }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

#define DISPINFO_SIZE 512

static void read_key_value(char* key_value, char* key, int* value) {
    int len = strlen(key_value);
    char buf[128];
    int p = 0;
    for (int i = 0; i < len; i++) {
        if (key_value[i] != ' ') buf[p++] = key_value[i];
    }
    buf[p] = '\0';
    sscanf(buf, "%[A-Za-z]:%d", key, value);
}

int NDL_Init(uint32_t flags) {
    char dispinfo[DISPINFO_SIZE];
    int fd = open("/proc/dispinfo", 0);
    read(fd, dispinfo, DISPINFO_SIZE);
    /*
    WIDTH   :  100
    HEIGHT  :  100
    */
    char key[128];
    int value = 0;
    char* key_value = strtok(dispinfo, "\n");
    while (key_value != NULL) {
        read_key_value(key_value, key, &value);
        if (strcmp(key, "WIDTH") == 0) {
            screen_w = value;
        } 
        else if (strcmp(key, "HEIGHT") == 0) {
            screen_h = value;
        }
        key_value = strtok(NULL, "\n");
    }
    printf("screen_w: %d, screen_h: %d\n", screen_w, screen_h);
    if (getenv("NWM_APP")) {
        evtdev = 3;
    }
    return 0;
}

void NDL_Quit() {
}
