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

void fw_cfg_dma_read_selector(uint16_t selector, void *buf, int len, int offset) {
    (void) offset;
    volatile struct fw_cfg_dma_access access = {0};
    access.command = bswap32(((uint32_t) selector << 16) | FW_CFG_DMA_CNT_READ | FW_CFG_DMA_CNT_SELECT);
    access.length = bswap32(len);
    access.address = bswap64((uint64_t) (uint32_t) buf);
    outd(FW_CFG_DMA + 4, bswap32((uint32_t) &access));
}

void fw_cfg_dma_write_selector(uint16_t selector, const void *buf, int len, int offset) {
    (void) offset;
    volatile struct fw_cfg_dma_access access = {0};
    access.command = bswap32(((uint32_t) selector << 16) | FW_CFG_DMA_CNT_WRITE | FW_CFG_DMA_CNT_SELECT);
    access.length = bswap32(len);
    access.address = bswap64((uint64_t) (uint32_t) buf);
    outd(FW_CFG_DMA + 4, bswap32((uint32_t) &access));
}
