#include <drivers/clock/rtc.h>
#include <motherboard/qemu/rtc_ext.h>

int qemu_rtc_ext_conv_mem_kb() {
    return ((int) rtc_read(QEMU_CMOS_CONV_MEM_HI) << 8) | rtc_read(QEMU_CMOS_CONV_MEM_LO);
}

int qemu_rtc_ext_ext1_mem_kb() {
    return ((int) rtc_read(QEMU_CMOS_EXT1_MEM_HI) << 8) | rtc_read(QEMU_CMOS_EXT1_MEM_LO);
}

int qemu_rtc_ext_ext2_mem_kb() {
    return (((int) rtc_read(QEMU_CMOS_EXT2_MEM_HI) << 8) | rtc_read(QEMU_CMOS_EXT2_MEM_LO)) * 65536 / 1024;
}

int qemu_rtc_ext_high_mem_kb() {
    return
          ((uint64_t) rtc_read(QEMU_CMOS_HIGH_MEM_HI) << 32)
        | ((uint64_t) rtc_read(QEMU_CMOS_HIGH_MEM_MI) << 24)
        | ((uint64_t) rtc_read(QEMU_CMOS_HIGH_MEM_LO) << 16)
        / 1024;
    ;
}
