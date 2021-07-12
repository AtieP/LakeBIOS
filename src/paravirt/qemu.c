#include <cpu/pio.h>
#include <paravirt/qemu.h>
#include <tools/bswap.h>
#include <tools/string.h>

int qemu_fw_cfg_get_file(char *filename, struct qemu_fw_cfg_file *info) {
    uint32_t items;
    size_t offset = 0;
    qemu_fw_cfg_read(QEMU_FW_CFG_SELECTOR_ROOT, &items, sizeof(uint32_t), offset);
    offset += sizeof(uint32_t);
    items = bswap32(items);
    for (; offset < items * sizeof(struct qemu_fw_cfg_file); offset += sizeof(struct qemu_fw_cfg_file)) {
        struct qemu_fw_cfg_file file;
        qemu_fw_cfg_read(QEMU_FW_CFG_SELECTOR_ROOT, &file, sizeof(struct qemu_fw_cfg_file), offset);
        if (!strcmp(file.name, filename)) {
            info->selector = bswap16(file.selector);
            info->size = bswap32(file.size);
            memcpy(&info->name, file.name, 56);
            return 0;
        }
    }
    return -1;
}

void qemu_fw_cfg_read(uint16_t selector, void *buf, size_t len, size_t offset) {
    volatile struct qemu_fw_cfg_cmd cmd;
    cmd.control = bswap32(((uint32_t) selector << 16) | QEMU_FW_CFG_CMD_SKIP | QEMU_FW_CFG_CMD_SELECT);
    cmd.length = bswap32(offset);
    outd(QEMU_PORT_DMA + 4, bswap32((uint32_t) &cmd));
    while (cmd.control & ~QEMU_FW_CFG_CMD_ERROR);
    cmd.control = bswap32(((uint32_t) selector << 16) | QEMU_FW_CFG_CMD_READ);
    cmd.length = bswap32(len);
    cmd.address = bswap64((uint64_t) (uintptr_t) buf);
    outd(QEMU_PORT_DMA + 4, bswap32((uint32_t) &cmd));
    while (cmd.control & ~QEMU_FW_CFG_CMD_ERROR);
}

void qemu_fw_cfg_write(uint16_t selector, const void *buf, size_t len, size_t offset) {
    volatile struct qemu_fw_cfg_cmd cmd;
    cmd.control = bswap32(((uint32_t) selector << 16) | QEMU_FW_CFG_CMD_SKIP | QEMU_FW_CFG_CMD_SELECT);
    cmd.length = bswap32(offset);
    outd(QEMU_PORT_DMA + 4, bswap32((uint32_t) &cmd));
    while (cmd.control & ~QEMU_FW_CFG_CMD_ERROR);
    cmd.control = bswap32(((uint32_t) selector << 16) | QEMU_FW_CFG_CMD_WRITE);
    cmd.length = bswap32(len);
    cmd.address = bswap64((uint64_t) (uintptr_t) buf);
    outd(QEMU_PORT_DMA + 4, bswap32((uint32_t) &cmd));
    while (cmd.control & ~QEMU_FW_CFG_CMD_ERROR);
}

