#ifndef __DRIVERS_CLOCK_RTC_H__
#define __DRIVERS_CLOCK_RTC_H__

#include <stdint.h>

#define RTC_INDEX 0x70
#define RTC_DATA 0x71
#define NMI_BIT (1 << 7)

#define CMOS_RESET_STATUS 0x0f

uint8_t rtc_read(uint8_t index);
void rtc_write(uint8_t index, uint8_t data);

uint8_t rtc_reset_status_get();
void rtc_reset_status_set(uint8_t status);

#endif
