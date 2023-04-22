#include <am.h>
#include <nemu.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = 0;

  uint32_t us_lo = *(uint32_t*)RTC_ADDR;
  uint32_t us_hi = *(uint32_t*)(RTC_ADDR + 1);

  uptime->us = ((uint64_t)us_hi >> 32) | us_lo;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
