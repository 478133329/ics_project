#include <am.h>
#include <nemu.h>

#define UPTIME_LOW_ADDR      (RTC_ADDR + 0x00)
#define UPTIME_HIGH_ADDR     (RTC_ADDR + 0x04)
#define REALTIME_LOW_ADDR    (RTC_ADDR + 0x08)
#define REALTIME_HIGH_ADDR   (RTC_ADDR + 0x0c)
#define RTC_SECOND_ADDR (RTC_ADDR + 0x10)
#define RTC_MINUTE_ADDR (RTC_ADDR + 0x14)
#define RTC_HOUR_ADDR   (RTC_ADDR + 0x18)
#define RTC_DAY_ADDR    (RTC_ADDR + 0x1c)
#define RTC_MONTH_ADDR  (RTC_ADDR + 0x20)
#define RTC_YEAR_ADDR   (RTC_ADDR + 0x24)

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T* uptime) {
	uptime->us = 0;
	// uint32_t us_lo = *(uint32_t*)UPTIME_ADDR;
	// uint32_t us_hi = *(uint32_t*)(UPTIME_ADDR + 4);
	uint32_t us_lo = inl(UPTIME_LOW_ADDR);
	uint32_t us_hi = inl(UPTIME_HIGH_ADDR);
	uptime->us = ((uint64_t)us_hi << 32) | us_lo;
}

void __am_timer_realtime(AM_TIMER_REALTIME_T* realtime) {
	realtime->us = 0;
	uint32_t us_lo = inl(REALTIME_LOW_ADDR);
	uint32_t us_hi = inl(REALTIME_HIGH_ADDR);
	realtime->us = ((uint64_t)us_hi << 32) | us_lo;
}

void __am_timer_rtc(AM_TIMER_RTC_T* rtc) {
	/*
	uint32_t us_lo = inl(REALTIME_LOW_ADDR);
	uint32_t us_hi = inl(REALTIME_HIGH_ADDR);
	uint64_t realtime = ((uint64_t)us_hi << 32) | us_lo;
	*/
	int32_t second = inl(RTC_SECOND_ADDR);
	int32_t minute = inl(RTC_MINUTE_ADDR);
	int32_t hour = inl(RTC_HOUR_ADDR);
	int32_t day = inl(RTC_DAY_ADDR);
	int32_t month = inl(RTC_MONTH_ADDR);
	int32_t year = inl(RTC_YEAR_ADDR);
	rtc->second = second;
	rtc->minute = minute;
	rtc->hour = hour;
	rtc->day = day;
	rtc->month = month;
	rtc->year = year;
}