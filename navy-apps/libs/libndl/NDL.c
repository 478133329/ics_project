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

static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0, canvas_x = 0, canvas_y = 0;

void NDL_OpenCanvas(int *w, int *h) {
    if (*w == 0 && *h == 0) {
        canvas_w = screen_w;
        canvas_h = screen_h;
        *w = canvas_w;
        *h = canvas_h;
    }
    else {
        if (*w > screen_w || *h > screen_h) {
            assert(0);
        }
        canvas_w = *w;
        canvas_h = *h;
    }
    canvas_x = (screen_w - canvas_w) / 2;
    canvas_y = (screen_h - canvas_h) / 2;

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

/*
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
    fbdev = open("/dev/fb", 0);
    for (int i = 0; i < h; i++) {
        lseek(fbdev, ((canvas_y + y + i) * screen_w + (canvas_x + x)) * sizeof(uint32_t), SEEK_SET);
        write(fbdev, pixels + i * w, w * sizeof(uint32_t));
    }
    close(fbdev);
}
*/

void NDL_DrawRect(uint32_t* pixels, int x, int y, int w, int h) {
    /*
    多次系统调用更新画面导致更新过慢，使用大缓存，调用一次write
    */
    int graphics = open("/dev/fb", O_RDWR);

    uint32_t* buf = (uint32_t*)malloc(sizeof(uint32_t) * screen_w * h);
    for (int i = 0; i < h; ++i) {
        memcpy(&buf[screen_w * i + canvas_x + x], &pixels[w * i], w * sizeof(uint32_t));
    }

    write(graphics, buf, sizeof(uint32_t) * screen_w * h);

    /*
    int graphics = open("/dev/fb", O_RDWR);
    for (int i = 0; i < h; ++i) {
        lseek(graphics, ((canvas_y + y + i) * screen_w + (canvas_x + x)) * sizeof(uint32_t), SEEK_SET);
        write(graphics, pixels + w * i, w * sizeof(uint32_t));
    }
    */
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
    char dispinfo[512];
    int fd = open("/proc/dispinfo", 0);
    read(fd, dispinfo, 512);
    /*
    WIDTH   :  100
    HEIGHT  :  100
    */
    char* key_value = strtok(dispinfo, "\n");
    while (key_value != NULL) {
        char key[128];
        int value = 0;
        read_key_value(key_value, key, &value);
        if (strcmp(key, "WIDTH") == 0) {
            screen_w = value;
        } 
        else if (strcmp(key, "HEIGHT") == 0) {
            screen_h = value;
        }
        key_value = strtok(NULL, "\n");
    }
    // printf("screen_w: %d, screen_h: %d\n", screen_w, screen_h);

    if (getenv("NWM_APP")) {
        evtdev = 3;
    }
    return 0;
}

void NDL_Quit() {
}
