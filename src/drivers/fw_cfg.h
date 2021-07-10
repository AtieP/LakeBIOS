#ifndef __DRIVERS_FW_CFG_H__
#define __DRIVERS_FW_CFG_H__

#include <stdint.h>

#define FW_CFG_SELECT 0x510
#define FW_CFG_DATA 0x511
#define FW_CFG_DMA 0x514

#define FW_CFG_SELECT_ROOT 0x19

#define FW_CFG_DMA_CNT_ERROR (1 << 0)
#define FW_CFG_DMA_CNT_READ (1 << 1)
#define FW_CFG_DMA_CNT_SKIP (1 << 2)
#define FW_CFG_DMA_CNT_SELECT (1 << 3)
#define FW_CFG_DMA_CNT_WRITE (1 << 4)

struct fw_cfg_file {
    uint32_t size;
    uint16_t selector;
    uint16_t reserved;
    char name[56];
} __attribute__((__packed__));

struct fw_cfg_dma_access {
    uint32_t command;
    uint32_t length;
    uint64_t address;
} __attribute__((__packed__));

struct fw_cfg_file fw_cfg_get_file(const char *filename);
void fw_cfg_read_selector(uint16_t selector, void *buf, int len);
void fw_cfg_write_selector(uint16_t selector, const void *buf, int len);
void fw_cfg_dma_read_selector(uint16_t selector, void *buf, int len, int offset);
void fw_cfg_dma_write_selector(uint16_t selector, const void *buf, int len, int offset);

#endif
