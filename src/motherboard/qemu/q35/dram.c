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
/*
    Notes about TSEG:

    * TSEG is just RAM that is only accessible while being in System Management Mode.
      The TSEG window is located at the Top of RAM at the 4GB area minus the size of TSEG.
    
    * QEMU is able to set custom TSEG sizes via a QEMU specific method.
    
      The custom TSEG size which the firmware can optionally set is passed via the command
      line and then is reported on the EXT_TSEG_MBYTES register (0x50, Word access). To query
      the size, the value EXT_TSEG_MBYTES_QUERY (0xffff) must be written to the register and then
      the register has to be read back. The custom size is in megabytes.

      In order to set the custom (extended) TSEG size as the TSEG size, the following three steps must
      be done:

      - SMRAM must be enabled via the SMRAM register.
      - TSEG must be enabled via the ESMRAMC register.
      - The TSEG size on the ESMRAMC register must be set to reserved (0x03).
*/

int qemu_q35_dram_tseg_get_current_size() {
    uint8_t smram = dram_read_byte(QEMU_Q35_DRAM_SMRAM);
    uint8_t esmramc = dram_read_byte(QEMU_Q35_DRAM_ESMRAMC);
    if (!(smram & QEMU_Q35_DRAM_SMRAM_EN) || !(esmramc & QEMU_Q35_DRAM_ESMRAMC_TSEG_EN)) {
        return 0;
    }
    uint8_t tseg_size = esmramc & QEMU_Q35_DRAM_ESMRAMC_TSEG_MASK;
    if (tseg_size == QEMU_Q35_DRAM_ESMRAMC_TSEG_1M) {
        return 1 * 1024 * 1024;
    } else if (tseg_size == QEMU_Q35_DRAM_ESMRAMC_TSEG_2M) {
        return 2 * 1024 * 1024;
    } else if (tseg_size == QEMU_Q35_DRAM_ESMRAMC_TSEG_8M) {
        return 8 * 1024 * 1024;
    } else {
        dram_write_word(QEMU_Q35_DRAM_EXT_TSEG_MBYTES, QEMU_Q35_DRAM_EXT_TSEG_MBYTES_QUERY);
        return (int) dram_read_word(QEMU_Q35_DRAM_EXT_TSEG_MBYTES) * 1024 * 1024;
    }
}

// Returned in MBs.
int qemu_q35_dram_tseg_get_extended_size() {
    dram_write_word(QEMU_Q35_DRAM_EXT_TSEG_MBYTES, QEMU_Q35_DRAM_EXT_TSEG_MBYTES_QUERY);
    return (int) dram_read_word(QEMU_Q35_DRAM_EXT_TSEG_MBYTES);
}

// A value of 0 disables TSEG.
int qemu_q35_dram_tseg_set_size(int mbs) {
    if (mbs == 0) {
        dram_write_byte(QEMU_Q35_DRAM_ESMRAMC, dram_read_byte(QEMU_Q35_DRAM_ESMRAMC) & ~QEMU_Q35_DRAM_ESMRAMC_TSEG_EN);
        return 0;
    } else if (mbs == 1) {
        dram_write_byte(QEMU_Q35_DRAM_ESMRAMC, (dram_read_byte(QEMU_Q35_DRAM_ESMRAMC) & ~QEMU_Q35_DRAM_ESMRAMC_TSEG_MASK) | QEMU_Q35_DRAM_ESMRAMC_TSEG_1M);
    } else if (mbs == 2) {
        dram_write_byte(QEMU_Q35_DRAM_ESMRAMC, (dram_read_byte(QEMU_Q35_DRAM_ESMRAMC) & ~QEMU_Q35_DRAM_ESMRAMC_TSEG_MASK) | QEMU_Q35_DRAM_ESMRAMC_TSEG_2M);
    } else if (mbs == 8) {
        dram_write_byte(QEMU_Q35_DRAM_ESMRAMC, (dram_read_byte(QEMU_Q35_DRAM_ESMRAMC) & ~QEMU_Q35_DRAM_ESMRAMC_TSEG_MASK) | QEMU_Q35_DRAM_ESMRAMC_TSEG_8M);
    } else if (mbs < QEMU_Q35_DRAM_EXT_TSEG_MBYTES_MAX) {
        if (mbs != qemu_q35_dram_tseg_get_extended_size()) {
            return -1;
        }
        dram_write_byte(QEMU_Q35_DRAM_ESMRAMC, (dram_read_byte(QEMU_Q35_DRAM_ESMRAMC) & ~QEMU_Q35_DRAM_ESMRAMC_TSEG_MASK) | QEMU_Q35_DRAM_ESMRAMC_TSEG_EXT);
    } else {
        return -1;
    }
    dram_write_byte(QEMU_Q35_DRAM_ESMRAMC, dram_read_byte(QEMU_Q35_DRAM_ESMRAMC) | QEMU_Q35_DRAM_ESMRAMC_TSEG_EN);
    return 0;
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

/* SMRAM control */

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

void qemu_q35_dram_esmramc_hi_smram_en() {
    dram_write_byte(QEMU_Q35_DRAM_ESMRAMC, dram_read_byte(QEMU_Q35_DRAM_ESMRAMC) | QEMU_Q35_DRAM_ESMRAMC_HI_SMRAM_EN);
}

void qemu_q35_dram_esmramc_hi_smram_dis() {
    dram_write_byte(QEMU_Q35_DRAM_ESMRAMC, dram_read_byte(QEMU_Q35_DRAM_ESMRAMC) &~ QEMU_Q35_DRAM_ESMRAMC_HI_SMRAM_EN);
}
