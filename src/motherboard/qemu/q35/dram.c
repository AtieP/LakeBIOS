#include <drivers/bus/pci.h>
#include <motherboard/qemu/q35/dram.h>

static inline uint8_t dram_read_byte(uint8_t offset) {
    return pci_cfg_read_byte(QEMU_Q35_DRAM_BUS, QEMU_Q35_DRAM_SLOT, QEMU_Q35_DRAM_FUNCTION, offset);
}

static inline uint16_t dram_read_word(uint8_t offset) {
    return pci_cfg_read_word(QEMU_Q35_DRAM_BUS, QEMU_Q35_DRAM_SLOT, QEMU_Q35_DRAM_FUNCTION, offset);
}

static inline uint32_t dram_read_dword(uint8_t offset) {
    return pci_cfg_read_dword(QEMU_Q35_DRAM_BUS, QEMU_Q35_DRAM_SLOT, QEMU_Q35_DRAM_FUNCTION, offset);
}

static inline void dram_write_byte(uint8_t offset, uint8_t value) {
    pci_cfg_write_byte(QEMU_Q35_DRAM_BUS, QEMU_Q35_DRAM_SLOT, QEMU_Q35_DRAM_FUNCTION, offset, value);
}

static inline void dram_write_word(uint8_t offset, uint16_t value) {
    pci_cfg_write_word(QEMU_Q35_DRAM_BUS, QEMU_Q35_DRAM_SLOT, QEMU_Q35_DRAM_FUNCTION, offset, value);   
}

static inline void dram_write_dword(uint8_t offset, uint32_t value) {
    pci_cfg_write_dword(QEMU_Q35_DRAM_BUS, QEMU_Q35_DRAM_SLOT, QEMU_Q35_DRAM_FUNCTION, offset, value);
}

/* TSEG */

int qemu_q35_dram_tseg_size() {
    uint8_t esmramc = dram_read_byte(QEMU_Q35_DRAM_ESMRAMC);
    if (!(esmramc & QEMU_Q35_DRAM_ESMRAMC_EN)) {
        return 0;
    }
    uint8_t esmramc_tseg_size = esmramc & 0x03;
    if (esmramc_tseg_size == QEMU_Q35_DRAM_ESMRAMC_TSEG_1M) {
        return 1 * 1024 * 1024;
    } else if (esmramc_tseg_size == QEMU_Q35_DRAM_ESMRAMC_TSEG_2M) {
        return 2 * 1024 * 1024;
    } else if (esmramc_tseg_size == QEMU_Q35_DRAM_ESMRAMC_TSEG_8M) {
        return 8 * 1024 * 1024;
    } else {
        dram_write_word(QEMU_Q35_DRAM_EXT_TSEG_MBYTES, QEMU_Q35_DRAM_EXT_TSEG_MBYTES_QUERY);
        return (int) dram_read_word(QEMU_Q35_DRAM_EXT_TSEG_MBYTES) * 1024 * 1024;
    }
}

/* PCIEXBAR */

void qemu_q35_dram_pciexbar(uint64_t pciexbar, int size_mb) {
    dram_write_dword(QEMU_Q35_DRAM_PCIEXBAR, (uint32_t) pciexbar | size_mb); // Disabling is implied
    dram_write_dword(QEMU_Q35_DRAM_PCIEXBAR + 4, (uint32_t) (pciexbar >> 32));
    dram_write_dword(QEMU_Q35_DRAM_PCIEXBAR, dram_read_dword(QEMU_Q35_DRAM_PCIEXBAR) | QEMU_Q35_DRAM_PCIEXBAR_EN);
}

/* Programmable Attribute Map */

void qemu_q35_dram_pam_lock(int pam) {
    dram_write_byte(QEMU_Q35_DRAM_PAM0 + pam, 0x00);
}

void qemu_q35_dram_pam_unlock(int pam) {
    dram_write_byte(QEMU_Q35_DRAM_PAM0 + pam, (QEMU_Q35_DRAM_PAM0_EN << 4) | (pam != 0 ? QEMU_Q35_DRAM_PAM0_EN : 0x00));
}

/* Legacy SMRAM control */

void qemu_q35_dram_smram_en() {
    dram_write_byte(QEMU_Q35_DRAM_SMRAM, dram_read_byte(QEMU_Q35_DRAM_SMRAM) | QEMU_Q35_DRAM_SMRAM_EN);
}

void qemu_q35_dram_smram_dis() {
    dram_write_byte(QEMU_Q35_DRAM_SMRAM, dram_read_byte(QEMU_Q35_DRAM_SMRAM) &~ QEMU_Q35_DRAM_SMRAM_EN);
}

void qemu_q35_dram_smram_open() {
    dram_write_byte(QEMU_Q35_DRAM_SMRAM, dram_read_byte(QEMU_Q35_DRAM_SMRAM) | QEMU_Q35_DRAM_SMRAM_OPEN);
}

void qemu_q35_dram_smram_close() {
    dram_write_byte(QEMU_Q35_DRAM_SMRAM, dram_read_byte(QEMU_Q35_DRAM_SMRAM) & ~(QEMU_Q35_DRAM_SMRAM_OPEN | QEMU_Q35_DRAM_SMRAM_CLOSE));
}

void qemu_q35_dram_smram_close_close() {
    dram_write_byte(QEMU_Q35_DRAM_SMRAM, (dram_read_byte(QEMU_Q35_DRAM_SMRAM) | QEMU_Q35_DRAM_SMRAM_CLOSE) & ~QEMU_Q35_DRAM_SMRAM_OPEN);
}

void qemu_q35_dram_smram_lock() {
    dram_write_byte(QEMU_Q35_DRAM_SMRAM, dram_read_byte(QEMU_Q35_DRAM_SMRAM) | QEMU_Q35_DRAM_SMRAM_LOCK);
}

/* Extended SMRAM Control */

void qemu_q35_dram_esmramc_en() {
    dram_write_byte(QEMU_Q35_DRAM_ESMRAMC, QEMU_Q35_DRAM_ESMRAMC_EN);
}

void qemu_q35_dram_esmramc_dis() {
    dram_write_byte(QEMU_Q35_DRAM_ESMRAMC, 0x00);
}
