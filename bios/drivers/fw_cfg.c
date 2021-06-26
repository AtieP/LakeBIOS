#include <cpu/pio.h>
#include <drivers/fw_cfg.h>
#include <tools/bswap.h>
#include <tools/string.h>

// todo: do not do this...
struct {
    uint32_t entries;
    struct fw_cfg_file files[32];
} __attribute__((__packed__)) files;

uint16_t fw_cfg_get_selector(const char *filename) {
    fw_cfg_dma_read_selector(FW_CFG_SELECT_ROOT, &files, sizeof(files), 0);
    for (int i = 0; i < 32; i++) {
        if (!strcmp(files.files[i].name, filename)) {
            return bswap16(files.files[i].selector);
        }
    }
    return 0;
}

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
    access.command = bswap32(((uint32_t) selector << 16) | FW_CFG_DMA_CNT_SKIP | FW_CFG_DMA_CNT_SELECT);
    access.length = offset;
    outd(FW_CFG_DMA + 4, bswap32((uint32_t) &access));
    access.command = bswap32(((uint32_t) selector << 16) | FW_CFG_DMA_CNT_READ | FW_CFG_DMA_CNT_SELECT);
    access.length = bswap32(len);
    access.address = bswap64((uint64_t) (uint32_t) buf);
    outd(FW_CFG_DMA + 4, bswap32((uint32_t) &access));
}

void fw_cfg_dma_write_selector(uint16_t selector, const void *buf, int len, int offset) {
    (void) offset;
    volatile struct fw_cfg_dma_access access = {0};
    access.command = bswap32(((uint32_t) selector << 16) | FW_CFG_DMA_CNT_SKIP | FW_CFG_DMA_CNT_SELECT);
    access.length = offset;
    outd(FW_CFG_DMA + 4, bswap32((uint32_t) &access));
    access.command = bswap32(((uint32_t) selector << 16) | FW_CFG_DMA_CNT_WRITE | FW_CFG_DMA_CNT_SELECT);
    access.length = bswap32(len);
    access.address = bswap64((uint64_t) (uint32_t) buf);
    outd(FW_CFG_DMA + 4, bswap32((uint32_t) &access));
}
