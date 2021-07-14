#ifndef __PARAVIRT_QEMU_H__
#define __PARAVIRT_QEMU_H__

#include <stddef.h>
#include <stdint.h>

#define QEMU_PORT_DMA 0x514

#define QEMU_FW_CFG_SELECTOR_ROOT 0x19

struct qemu_fw_cfg_file {
    uint32_t size;
    uint16_t selector;
    uint16_t reserved;
    char name[56];
} __attribute__((__packed__));

#define QEMU_FW_CFG_CMD_ERROR (1 << 0)
#define QEMU_FW_CFG_CMD_READ (1 << 1)
#define QEMU_FW_CFG_CMD_SKIP (1 << 2)
#define QEMU_FW_CFG_CMD_SELECT (1 << 3)
#define QEMU_FW_CFG_CMD_WRITE (1 << 4)
struct qemu_fw_cfg_cmd {
    uint32_t control;
    uint32_t length;
    uint64_t address;
} __attribute__((__packed__));

// All input and output data must be little endian.
int qemu_fw_cfg_get_file(char *filename, struct qemu_fw_cfg_file *info);
void qemu_fw_cfg_read(uint16_t selector, void *buf, size_t len, size_t offset);
void qemu_fw_cfg_write(uint16_t selector, const void *buf, size_t len, size_t offset);
int qemu_ramfb_detect();
int qemu_ramfb_resolution(uint64_t fb, uint32_t width, uint32_t height, uint32_t bpp);

#endif
