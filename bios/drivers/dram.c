#include <drivers/dram.h>
#include <drivers/pci.h>
#include <tools/print.h>
#include <tools/string.h>

extern char bios_code_start[];
extern char bios_code_end[];

static void dram_unlock_bios_internal() {
    // Get PAM values
    uint8_t pams[7];
    for (int i = 0; i <= 6; i++) {
        pams[i] = pci_cfg_read_byte(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_PAM0 + i);
    }
    // Populate them
    for (int i = 0; i <= 6; i++) {
        pams[i] |= DRAM_PAM_HI_DRAM;
        if (i != 0) {
            pams[i] |= DRAM_PAM_LO_DRAM;
        }
    }
    // Finally flush them
    for (int i = 0; i <= 6; i++) {
        pci_cfg_write_byte(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_PAM0 + i, pams[i]);
    }
    // Need to memcpy the BIOS into RAM
    memcpy(bios_code_start, bios_code_start + 0xfff00000, bios_code_end - bios_code_start);
}

// Reads and writes now go to DMI
void dram_lock_bios() {
    print("atiebios: dram: locking BIOS");
    // Get PAM values
    uint8_t pams[7];
    for (int i = 0; i <= 6; i++) {
        pams[i] = pci_cfg_read_byte(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_PAM0 + i);
    }
    // Populate them
    for (int i = 0; i <= 6; i++) {
        pams[i] &= ~DRAM_PAM_HI_DRAM;
        if (i != 0) {
            pams[i] &= ~DRAM_PAM_LO_DRAM;
        }
    }
    // Finally flush them
    for (int i = 0; i <= 6; i++) {
        pci_cfg_write_byte(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_PAM0 + i, pams[i]);
    }
    print("atiebios: dram: finished locking BIOS");
}

// Reads and writes now go to DRAM
void dram_unlock_bios() {
    print("atiebios: dram: unlocking BIOS");
    // QEMU seems to be mirroring the BIOS at 0xfff00000, so
    // to solve the bootstrapping problem call the unlocker from there
    void (*function)() = (void (*)()) ((uintptr_t) dram_unlock_bios_internal + 0xfff00000);
    function();
    print("atiebios: dram: finished unlocking BIOS");
}

// Open the SMM window
void dram_open_smram() {
    pci_cfg_write_byte(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_SMRAM, (pci_cfg_read_byte(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_SMRAM) & ~DRAM_SMRAM_CLOSED) | DRAM_SMRAM_OPEN | DRAM_SMRAM_DEFAULT);
}

// Closes the SMM window
void dram_close_smram() {
    pci_cfg_write_byte(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_SMRAM, (pci_cfg_read_byte(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_SMRAM) & ~DRAM_SMRAM_OPEN) | DRAM_SMRAM_CLOSED | DRAM_SMRAM_DEFAULT);
}

// Locks (applies) the SMM window
void dram_lock_smram() {
    pci_cfg_write_byte(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_SMRAM, pci_cfg_read_byte(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_SMRAM) | DRAM_SMRAM_LOCKED | DRAM_SMRAM_DEFAULT);
}

// Enables the SMM window for SMIs
void dram_enable_smram() {
    pci_cfg_write_byte(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_SMRAM, pci_cfg_read_byte(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_SMRAM) | DRAM_SMRAM_ENABLE | DRAM_SMRAM_DEFAULT);
}

// Enable the PCIEXBAR to enable ECAM
void dram_enable_pciexbar() {
    pci_cfg_write_dword(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_PCIEXBAR, pci_cfg_read_dword(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_PCIEXBAR) | DRAM_PCIEXBAR_ENABLE);
}

// Disable PCIEXBAR
void dram_disable_pciexbar() {
    pci_cfg_write_dword(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_PCIEXBAR, pci_cfg_read_dword(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_PCIEXBAR) & ~DRAM_PCIEXBAR_ENABLE);
}

// Set the PCIEXBAR address
void dram_set_pciexbar(uint64_t base) {
    pci_cfg_write_dword(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_PCIEXBAR, 0);
    pci_cfg_write_dword(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_PCIEXBAR + 4, base >> 32);
    pci_cfg_write_dword(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_PCIEXBAR, (base & 0xffffffff) | 1);
}

// Set the top of usable memory of lower memory (4gb)
// This function is needed to unmark PCIe ECAM and BARs as RAM
void dram_set_tolud(uint32_t tolud) {
    pci_cfg_write_word(DRAM_BUS, DRAM_SLOT, DRAM_FUNCTION, DRAM_TOLUD, (tolud >> 16) & ~(0b1111));
}
