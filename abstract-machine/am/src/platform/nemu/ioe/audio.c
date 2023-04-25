#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)
#define AUDIO_FRONT_ADDR     (AUDIO_ADDR + 0x18)
#define AUDIO_REAR_ADDR      (AUDIO_ADDR + 0x1c)

/*
AM_DEVREG(14, AUDIO_CONFIG, RD, bool present; int bufsize);
AM_DEVREG(15, AUDIO_CTRL,   WR, int freq, channels, samples);
AM_DEVREG(16, AUDIO_STATUS, RD, int count, front, rear);
AM_DEVREG(17, AUDIO_PLAY,   WR, Area buf);
*/

void __am_audio_init() {
	outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
	cfg->present = true;
	cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
	outl(AUDIO_FREQ_ADDR, ctrl->freq);
	outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
	outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
	__am_audio_init();
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
	// 由于copy，导致找了三个小时bug。
	stat->count = inl(AUDIO_COUNT_ADDR);
	stat->front = inl(AUDIO_FRONT_ADDR);
	stat->rear = inl(AUDIO_REAR_ADDR);
}

int printf(const char* fmt, ...);

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
	int32_t buf_size = io_read(AM_AUDIO_CONFIG).bufsize;
	int32_t full = io_read(AM_AUDIO_STATUS).count;
	int32_t empty = buf_size - full;
	int32_t need = ctl->buf.end - ctl->buf.start;
	volatile int32_t rear = io_read(AM_AUDIO_STATUS).rear;
	while (empty < need);
	for (int i = 0; i < need; i++) {
		outw(AUDIO_SBUF_ADDR + rear + (2 * i), ((int16_t*)(ctl->buf.start))[i]);
	}
}

// io_read(AM_AUDIO_CONFIG).present
// io_write(AM_AUDIO_CTRL, 8000, 1, 1024)
// io_write(AM_AUDIO_PLAY, sbuf)
// while (io_read(AM_AUDIO_STATUS).count > 0);
