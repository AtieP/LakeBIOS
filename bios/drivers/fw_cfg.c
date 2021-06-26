#include <cpu/pio.h>
#include <drivers/fw_cfg.h>
#include <tools/bswap.h>

void fw_cfg_read_selector(uint16_t selector, void *buf, int len) {
    uint8_t *buf_uint8 = (uint8_t *) buf;
    outw(FW_CFG_SELECT, selector);
    for (int i = 0; i < len; i++) {
        buf_uint8[i] = inb(FW_CFG_DATA);
    }
}

void fw_cfg_write_selector(uint16_t selector, const void *buf, int len) {
    const uint8_t *buf_uint8 = (const uint8_t *) buf;
    outw(FW_CFG_SELECT, selector);
    for (int i = 0; i < len; i++) {
        outb(FW_CFG_DATA, buf_uint8[i]);
    }
}
