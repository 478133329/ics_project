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

#include <device/map.h>
#include <device/alarm.h>
#include <utils.h>

#include <time.h>

static uint32_t *rtc_port_base = NULL;

static void rtc_io_handler(uint32_t offset, int len, bool is_write) {
    assert(offset == 0 || offset == 4 || offset == 8 || offset == 12 || offset == 16 || offset == 20 || offset == 24 || offset == 28 || offset == 32 || offset == 36);
    if (!is_write && offset == 0) {
        uint64_t us = get_uptime();
        rtc_port_base[0] = (uint32_t)us;
        rtc_port_base[1] = us >> 32;
    }
    else if (!is_write == 0 && offset == 8) {
        uint64_t us = get_realtime();
        rtc_port_base[2] = (uint32_t)us;
        rtc_port_base[3] = us >> 32;
    }
    else if (!is_write == 0 && offset == 16) {
        uint64_t us = get_realtime();
        time_t t = (time_t)us;
        time_t* time = &t;
        struct tm* ptm = gmtime(time);
        rtc_port_base[4] = ptm->tm_sec;
        rtc_port_base[5] = ptm->tm_min;
        rtc_port_base[6] = ptm->tm_hour;
        rtc_port_base[7] = ptm->tm_mday;
        rtc_port_base[8] = ptm->tm_mon;
        rtc_port_base[9] = ptm->tm_year;
    }
}

#ifndef CONFIG_TARGET_AM
static void timer_intr() {
  if (nemu_state.state == NEMU_RUNNING) {
    extern void dev_raise_intr();
    dev_raise_intr();
  }
}
#endif

void init_timer() {
  rtc_port_base = (uint32_t *)new_space(40);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("rtc", CONFIG_RTC_PORT, rtc_port_base, 8, rtc_io_handler);
#else
  add_mmio_map("rtc", CONFIG_RTC_MMIO, rtc_port_base, 8, rtc_io_handler);
#endif
  IFNDEF(CONFIG_TARGET_AM, add_alarm_handle(timer_intr));
}
