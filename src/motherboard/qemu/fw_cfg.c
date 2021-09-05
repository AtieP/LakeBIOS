#include <cpu/misc.h>
#include <cpu/pio.h>
#include <motherboard/qemu/fw_cfg.h>
#include <tools/bswap.h>
#include <tools/string.h>

static int dma_rw(uint16_t selector, void *buf, uint32_t size, uint32_t offset, int write) {
    volatile struct qemu_fw_cfg_dma_packet dma_packet = {0};
    dma_packet.control = bswap32(((uint32_t) selector << 16) | QEMU_FW_CFG_DMA_OP_SELECT | QEMU_FW_CFG_DMA_OP_SKIP);
    dma_packet.length = bswap32(offset);
    outd(QEMU_FW_CFG_DMA_PORT + 4, bswap32((uint32_t) (uintptr_t) &dma_packet));
    while (dma_packet.control & ~QEMU_FW_CFG_DMA_OP_ERR) {
        pause();
    }
    dma_packet.control = bswap32(write ? QEMU_FW_CFG_DMA_OP_WRITE : QEMU_FW_CFG_DMA_OP_READ);
    dma_packet.length = bswap32(size);
    dma_packet.address = bswap64((uint64_t) (uintptr_t) buf);
    outd(QEMU_FW_CFG_DMA_PORT + 4, bswap32((uint32_t) (uintptr_t) &dma_packet));
    while (dma_packet.control & ~QEMU_FW_CFG_DMA_OP_ERR) {
        pause();
    }
    return 0;
}

int qemu_fw_cfg_detect() {
    char qemu[4] = {0};
    qemu_fw_cfg_read_sel(QEMU_FW_CFG_SIGNATURE, &qemu, 4, 0);
    if (!strncmp(qemu, "QEMU", 4)) {
        return 1;
    }
    return 0;
}

int qemu_fw_cfg_get_entry(const char *name, struct qemu_fw_cfg_entry *entry, size_t index) {
    uint32_t offset = 0;
    uint32_t entries;
    size_t i = 0;
    qemu_fw_cfg_read_sel(QEMU_FW_CFG_ROOT_DIR, &entries, sizeof(uint32_t), offset);
    offset += sizeof(uint32_t);
    entries = bswap32(entries);
    for (; offset < entries * sizeof(struct qemu_fw_cfg_entry); offset += sizeof(struct qemu_fw_cfg_entry)) {
        qemu_fw_cfg_read_sel(QEMU_FW_CFG_ROOT_DIR, entry, sizeof(struct qemu_fw_cfg_entry), offset);
        if (!strcmp(entry->name, name)) {
            if (i == index) {
                entry->select = bswap16(entry->select);
                entry->size = bswap32(entry->size);
                return 0;
            }
        }
    }
    return -1;
}

int qemu_fw_cfg_read_sel(uint16_t selector, void *buf, uint32_t size, uint32_t offset) {
    return dma_rw(selector, buf, size, offset, 0);
}

int qemu_fw_cfg_write_sel(uint16_t selector, void *buf, uint32_t size, uint32_t offset) {
    return dma_rw(selector, buf, size, offset, 1);
}

int qemu_fw_cfg_read_raw(struct qemu_fw_cfg_entry *entry, void *buf, uint32_t size, uint32_t offset) {
    if (entry->size > size + offset) {
        return -1;
    }
    return dma_rw(entry->select, buf, size, offset, 0);
}

int qemu_fw_cfg_write_raw(struct qemu_fw_cfg_entry *entry, void *buf, uint32_t size, uint32_t offset) {
    if (entry->size > size + offset) {
        return -1;
    }
    return dma_rw(entry->select, buf, size, offset, 1);
}

int qemu_fw_cfg_read_int(struct qemu_fw_cfg_entry *entry, uint64_t *integer) {
    if (entry->size != 8) {
        return -1;
    }
    return dma_rw(entry->select, integer, 8, 0, 0);
}
