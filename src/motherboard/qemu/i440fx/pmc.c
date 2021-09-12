#include <stdint.h>
#include <drivers/bus/pci.h>
#include <motherboard/qemu/i440fx/pmc.h>

static inline uint8_t pmc_read_byte(uint8_t offset) {
    return pci_cfg_read_byte(QEMU_I440FX_PMC_BUS, QEMU_I440FX_PMC_SLOT, QEMU_I440FX_PMC_FUNCTION, offset);
}

static inline void pmc_write_byte(uint8_t offset, uint8_t value) {
    pci_cfg_write_byte(QEMU_I440FX_PMC_BUS, QEMU_I440FX_PMC_SLOT, QEMU_I440FX_PMC_FUNCTION, offset, value);
}

/* Programmable Attribute Map */

void qemu_i440fx_pmc_pam_lock(int pam) {
    pmc_write_byte(QEMU_I440FX_PMC_PAM0 + pam, 0);
}

void qemu_i440fx_pmc_pam_unlock(int pam) {
    pmc_write_byte(QEMU_I440FX_PMC_PAM0 + pam, QEMU_I440FX_PMC_PAM_EN_HI | (pam != 0 ? QEMU_I440FX_PMC_PAM_EN_LO : 0));
}

/* SMRAM Control */

void qemu_i440fx_pmc_smram_en() {
    pmc_write_byte(QEMU_I440FX_PMC_SMRAM, (pmc_read_byte(QEMU_I440FX_PMC_SMRAM) & ~0x07) | QEMU_I440FX_PMC_SMRAM_EN | QEMU_I440FX_PMC_SMRAM_DBASESEG);
}

void qemu_i440fx_pmc_smram_dis() {
    pmc_write_byte(QEMU_I440FX_PMC_SMRAM, pmc_read_byte(QEMU_I440FX_PMC_SMRAM) & ~QEMU_I440FX_PMC_SMRAM_EN);
}

void qemu_i440fx_pmc_smram_open() {
    pmc_write_byte(QEMU_I440FX_PMC_SMRAM, (pmc_read_byte(QEMU_I440FX_PMC_SMRAM) & ~QEMU_I440FX_PMC_SMRAM_CLOSE) | QEMU_I440FX_PMC_SMRAM_OPEN);
}

void qemu_i440fx_pmc_smram_close() {
    pmc_write_byte(QEMU_I440FX_PMC_SMRAM, pmc_read_byte(QEMU_I440FX_PMC_SMRAM) & ~QEMU_I440FX_PMC_SMRAM_OPEN);
}

void qemu_i440fx_pmc_smram_close_close() {
    pmc_write_byte(QEMU_I440FX_PMC_SMRAM, (pmc_read_byte(QEMU_I440FX_PMC_SMRAM) & ~QEMU_I440FX_PMC_SMRAM_OPEN) | QEMU_I440FX_PMC_SMRAM_CLOSE);
}

void qemu_i440fx_pmc_smram_lock() {
    pmc_write_byte(QEMU_I440FX_PMC_SMRAM, pmc_read_byte(QEMU_I440FX_PMC_SMRAM) | QEMU_I440FX_PMC_SMRAM_LOCK);
}
