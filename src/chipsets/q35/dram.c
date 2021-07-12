#include <chipsets/q35.h>
#include <drivers/pci.h>
#include <tools/string.h>

extern char bios_raw_start[];
extern char bios_raw_end[];

// BIOS shadowing functions

static void pam_unlock_internal(int pam, int copy_to_ram) {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PAM_BASE + pam, pam != 0 ? (Q35_DRAM_PAM_UNLOCK << 4) | Q35_DRAM_PAM_UNLOCK : Q35_DRAM_PAM_UNLOCK << 4);
    if (copy_to_ram) {
        memcpy(bios_raw_start, 0xfff00000 + bios_raw_start, bios_raw_end - bios_raw_start);
    }
}

void q35_dram_pam_lock(int pam) {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PAM_BASE + pam, 0);
}

void q35_dram_pam_unlock(int pam, int copy_ram) {
    void (*function)(int, int) = (void (*)(int, int)) (uintptr_t) pam_unlock_internal;
    function(pam, copy_ram);
}

// SMRAM management

void q35_dram_smram_open() {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM, Q35_DRAM_SMRAM_OPEN | Q35_DRAM_SMRAM_ENABLE);
}

void q35_dram_smram_close() {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM, Q35_DRAM_SMRAM_CLOSE | Q35_DRAM_SMRAM_ENABLE);
}

void q35_dram_smram_lock() {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM, Q35_DRAM_SMRAM_CLOSE | Q35_DRAM_SMRAM_LOCK | Q35_DRAM_SMRAM_ENABLE);
}

// PCIEXBAR

void q35_dram_pciexbar_set(uint64_t pciexbar) {
    pci_cfg_write_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR,
        pci_cfg_read_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR) | (pciexbar & 0xffffffff));
    pci_cfg_write_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR + 4, pciexbar >> 32);
}

void q35_dram_pciexbar_enable() {
    pci_cfg_write_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR,
        pci_cfg_read_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR) | Q35_DRAM_PCIEXBAR_EN);
}

void q35_dram_pciexbar_disable() {
    pci_cfg_write_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR,
        pci_cfg_read_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR) & ~Q35_DRAM_PCIEXBAR_EN);
}

// Others
void q35_dram_tolud(uint32_t tolud) {
    pci_cfg_write_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_TOLUD, tolud);
}
