#ifndef __MOTHERBOARD_QEMU_RTC_EXT_H__
#define __MOTHERBOARD_QEMU_RTC_EXT_H__

#define QEMU_CMOS_CONV_MEM_LO 0x15
#define QEMU_CMOS_CONV_MEM_HI 0x16
#define QEMU_CMOS_EXT1_MEM_LO 0x30
#define QEMU_CMOS_EXT1_MEM_HI 0x31
#define QEMU_CMOS_EXT2_MEM_LO 0x34
#define QEMU_CMOS_EXT2_MEM_HI 0x35
#define QEMU_CMOS_HIGH_MEM_LO 0x5b
#define QEMU_CMOS_HIGH_MEM_MI 0x5c
#define QEMU_CMOS_HIGH_MEM_HI 0x5d

int qemu_rtc_ext_conv_mem_kb(); // 0-640KB
int qemu_rtc_ext_ext1_mem_kb(); // 1M-16M
int qemu_rtc_ext_ext2_mem_kb(); // 16M-4GB
int qemu_rtc_ext_high_mem_kb(); // 4GB-?????GB

#endif
