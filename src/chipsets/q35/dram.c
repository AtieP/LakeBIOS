#include <chipsets/q35.h>
#include <chipsets/q35/dram.h>
#include <drivers/pci.h>
#include <tools/string.h>

extern char bios_raw_start[];
extern char bios_raw_end[];

void q35_dram_pam_lock(int pam) {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PAM0 + pam, 0);
}

void q35_dram_pam_unlock(int pam, int copy_to_ram) {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PAM0 + pam, pam == 0 ? Q35_DRAM_PAM_UNLOCK << 4 : (Q35_DRAM_PAM_UNLOCK << 4) | Q35_DRAM_PAM_UNLOCK);
    if (copy_to_ram) {
        memcpy(bios_raw_start, 0xfff00000 + bios_raw_start, bios_raw_end - bios_raw_start);
    }
}

void q35_dram_smram_open() {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM, Q35_DRAM_SMRAM_OPEN | Q35_DRAM_SMRAM_EN);
}

void q35_dram_smram_close() {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM, Q35_DRAM_SMRAM_CLOSE | Q35_DRAM_SMRAM_EN);
}

void q35_dram_smram_lock() {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM, Q35_DRAM_SMRAM_CLOSE | Q35_DRAM_SMRAM_EN | Q35_DRAM_SMRAM_LOCK);
}
