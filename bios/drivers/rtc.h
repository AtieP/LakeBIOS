#ifndef __DRIVERS_RTC_H__
#define __DRIVERS_RTC_H__

#include <stdint.h>

#define RTC_INDEX 0x70
#define RTC_DATA 0x71

#define NMI_BIT (1 << 7)

#define CMOS_FLOPPY 0x10
#define CMOS_FLOPPY_NO 0x00
#define CMOS_FLOPPY_360 0x01
#define CMOS_FLOPPY_1200 0x02
#define CMOS_FLOPPY_720 0x03
#define CMOS_FLOPPY_1440 0x04
#define CMOS_FLOPPY_2880 0x05

// Low memory: memory between 0M and 1M
#define CMOS_LOWMEM_LO 0x15
#define CMOS_LOWMEM_HI 0x16

// Extended memory 1: memory between 1M and 16M
#define CMOS_EXTMEM1_LO 0x30
#define CMOS_EXTMEM1_HI 0x31

// Extended memory 2: memory between 16M and 4G
#define CMOS_EXTMEM2_LO 0x34
#define CMOS_EXTMEM2_HI 0x35

uint8_t rtc_read(uint8_t index);
void rtc_write(uint8_t index, uint8_t data);

uint8_t rtc_get_floppy_master_kind();
uint8_t rtc_get_floppy_slave_kind();
int rtc_get_low_mem();
int rtc_get_ext1_mem();
int rtc_get_ext2_mem();

#endif
