#ifndef __MOTHERBOARD_QEMU_FW_CFG_H__
#define __MOTHERBOARD_QEMU_FW_CFG_H__

#include <stddef.h>
#include <stdint.h>

#define QEMU_FW_CFG_DMA_PORT 0x514

#define QEMU_FW_CFG_SIGNATURE 0x0000
#define QEMU_FW_CFG_ROOT_DIR  0x0019

struct qemu_fw_cfg_entry {
    uint32_t size;
    uint16_t select;
    uint16_t reserved;
    char name[56];
} __attribute__((__packed__));

#define QEMU_FW_CFG_DMA_OP_ERR     1
#define QEMU_FW_CFG_DMA_OP_READ    2
#define QEMU_FW_CFG_DMA_OP_SKIP    4
#define QEMU_FW_CFG_DMA_OP_SELECT  8
#define QEMU_FW_CFG_DMA_OP_WRITE  16

struct qemu_fw_cfg_dma_packet {
    uint32_t control;
    uint32_t length;
    uint64_t address;
} __attribute__((__packed__));

int qemu_fw_cfg_detect();
int qemu_fw_cfg_get_entry(const char *name, struct qemu_fw_cfg_entry *entry, size_t index);
int qemu_fw_cfg_read_sel(uint16_t selector, void *buf, uint32_t size, uint32_t offset);
int qemu_fw_cfg_write_sel(uint16_t selector, void *buf, uint32_t size, uint32_t offset);
int qemu_fw_cfg_read_raw(struct qemu_fw_cfg_entry *entry, void *buf, uint32_t size, uint32_t offset);
int qemu_fw_cfg_write_raw(struct qemu_fw_cfg_entry *entry, void *buf, uint32_t size, uint32_t offset);
int qemu_fw_cfg_read_int(struct qemu_fw_cfg_entry *entry, uint64_t *integer);

#endif
