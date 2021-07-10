#include <cpu/pio.h>
#include <drivers/fw_cfg.h>
#include <tools/bswap.h>
#include <tools/string.h>

// Returns a file from the fw_cfg root directory.
// Returns 0 in selector if not found.
// Length and selectors are byte swapped to little endian.
struct fw_cfg_file fw_cfg_get_file(const char *filename) {
    struct fw_cfg_file file;
    uint32_t entries;
    uint32_t offset = 0;
    fw_cfg_dma_read_selector(FW_CFG_SELECT_ROOT, &entries, sizeof(uint32_t), offset);
    offset += 4;
    entries = bswap32(entries);
    for (; offset < entries * sizeof(struct fw_cfg_file); offset += sizeof(struct fw_cfg_file)) {
        fw_cfg_dma_read_selector(FW_CFG_SELECT_ROOT, &file, sizeof(struct fw_cfg_file), offset);
        if (!strcmp(file.name, filename)) {
            file.selector = bswap16(file.selector);
            file.size = bswap32(file.size);
            return file;
        }
    }
    file.selector = 0;
    return file;
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
    access.length = bswap32(offset);
    outd(FW_CFG_DMA + 4, bswap32((uint32_t) &access));
    while (access.command & ~FW_CFG_DMA_CNT_ERROR);
    access.command = bswap32(((uint32_t) selector << 16) | FW_CFG_DMA_CNT_READ);
    access.length = bswap32(len);
    access.address = bswap64((uint64_t) (uint32_t) buf);
    outd(FW_CFG_DMA + 4, bswap32((uint32_t) &access));
    while (access.command & ~FW_CFG_DMA_CNT_ERROR);
}

void fw_cfg_dma_write_selector(uint16_t selector, const void *buf, int len, int offset) {
    (void) offset;
    volatile struct fw_cfg_dma_access access = {0};
    access.command = bswap32(((uint32_t) selector << 16) | FW_CFG_DMA_CNT_SKIP | FW_CFG_DMA_CNT_SELECT);
    access.length = bswap32(offset);
    outd(FW_CFG_DMA + 4, bswap32((uint32_t) &access));
    while (access.command & ~FW_CFG_DMA_CNT_ERROR);
    access.command = bswap32(((uint32_t) selector << 16) | FW_CFG_DMA_CNT_WRITE);
    access.length = bswap32(len);
    access.address = bswap64((uint64_t) (uint32_t) buf);
    outd(FW_CFG_DMA + 4, bswap32((uint32_t) &access));
    while (access.command & ~FW_CFG_DMA_CNT_ERROR);
}
