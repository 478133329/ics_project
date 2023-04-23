#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
    int i;
    int w = io_read(AM_GPU_CONFIG).width;  // TODO: get the correct width
    int h = io_read(AM_GPU_CONFIG).height;  // TODO: get the correct height
    uint32_t * fb = (uint32_t*)(uintptr_t)FB_ADDR;
    for (i = 0; i < w * h; i++) fb[i] = i;
    outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    uint32_t config = inl(VGACTL_ADDR);
    int32_t width = config >> 16;
    int32_t height = config & 0xffff;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = 0
  };
}

// #define io_read(reg)
//  ({ reg##_T __io_param;
//    ioe_read(reg, &__io_param);
//    __io_param; })

// #define io_write(reg, ...)
//  ({ reg##_T __io_param = (reg##_T) { __VA_ARGS__ };
//    ioe_write(reg, &__io_param); })

// void ioe_read (int reg, void *buf) { ((handler_t)lut[reg])(buf); }
// void ioe_write(int reg, void* buf) { ((handler_t)lut[reg])(buf); }

// AM_DEVREG( 9, GPU_CONFIG, RD, bool present, has_accel; int width, height, vmemsz);
// AM_DEVREG(10, GPU_STATUS, RD, bool ready);
// AM_DEVREG(11, GPU_FBDRAW, WR, int x, y; void* pixels; int w, h; bool sync);

// static void* lut[128] = {
//  [AM_GPU_CONFIG] = __am_gpu_config,
//  [AM_GPU_FBDRAW] = __am_gpu_fbdraw,
//  [AM_GPU_STATUS] = __am_gpu_status,
//};

// AM_DEVREG(11, GPU_FBDRAW,   WR, int x, y; void *pixels; int w, h; bool sync);
#include <stdio.h>
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int window_width = io_read(AM_GPU_CONFIG).width;
  int x = ctl->x;
  int y = ctl->y;
  int w = ctl->w;
  int h = ctl->h;
  for (int row = 0; row < h; row++) {
      for (int col = 0; col < w; col++) {
          // ((uint32_t**)FB_ADDR)[row + y][col + x] = ((uint32_t**)ctl->pixels)[row][col]; 不能直接将一个指针当作二维数组使用，因为不知道每行有几个元素
          ((uint32_t*)FB_ADDR)[(y + row) * window_width + x + col] = ((uint32_t*)ctl->pixels)[row * w + col];
      }
  }
  if (ctl->sync) {
      outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
