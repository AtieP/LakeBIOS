#include <cpu/pio.h>
#include <drivers/clock/rtc.h>

uint8_t rtc_read(uint8_t index) {
    outb(RTC_INDEX, index | NMI_BIT);
    return inb(RTC_DATA);
}

void rtc_write(uint8_t index, uint8_t data) {
    outb(RTC_INDEX, index | NMI_BIT);
    outb(RTC_DATA, data);
}

uint8_t rtc_reset_status_get() {
    return rtc_read(CMOS_RESET_STATUS);
}

void rtc_reset_status_set(uint8_t status) {
    rtc_write(CMOS_RESET_STATUS, status);
}
