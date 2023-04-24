/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

#define BUF_FRONT audio_base[reg_front]
#define BUF_REAR audio_base[reg_rear]

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  reg_front,
  reg_rear,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

void audioCallback(void* userdata, Uint8* stream, int len) {
	if (audio_base[reg_count] == 0) return;
	len = (len < audio_base[reg_count]) ? len : audio_base[reg_count];
	intptr_t addr = CONFIG_SB_ADDR + BUF_FRONT;
	SDL_memcpy(stream, (char*)addr, len);
	BUF_FRONT += len;
	audio_base[reg_count] -= len;
}

void init_audio_dev() {
	SDL_AudioSpec s = {};
	s.freq = audio_base[reg_freq];
	s.channels = audio_base[reg_channels];
	s.samples = audio_base[reg_samples];
	s.format = AUDIO_S16SYS;  // 假设系统中音频数据的格式总是使用16位有符号数来表示
	s.userdata = NULL;
	s.callback = audioCallback;
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	SDL_OpenAudio(&s, NULL);
	SDL_PauseAudio(0);
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
	if (offset == 16) init_audio_dev();
}

static void sb_io_handler(uint32_t offset, int len, bool is_write) {
	assert(is_write && len == 2);
	while (len--) {
		BUF_REAR = (BUF_REAR + 1) % CONFIG_SB_SIZE;
		if (BUF_FRONT == BUF_REAR) panic("sb queue over.\n");
		audio_base[reg_count]++;
	}
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
  audio_base[reg_count] = 0;
  audio_base[reg_front] = 0;
  audio_base[reg_rear] = 0;
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, sb_io_handler);
}
