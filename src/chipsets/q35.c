#include <chipsets/q35.h>
#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/pci.h>
#include <tools/print.h>
#include <tools/string.h>

extern char bios_raw_start[];
extern char bios_raw_end[];

static void unshadow_bios_internal(int first_pam) {
    uint8_t pam;
    for (int i = first_pam; i < 7; i++) {
        pam = pci_cfg_read_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PAM_BASE + i);
        pam |= 0b11 << 4;
        if (i != 0) {
            pam |= 0b11;
        }
        pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PAM_BASE + i, pam);
    }
    // Copy now the BIOS into ram
    memcpy(bios_raw_start, bios_raw_start + 0xfff00000, bios_raw_end - bios_raw_start - (first_pam * 0x10000));
}

void q35_dram_shadow_bios(int first_pam) {
    for (int i = first_pam; i < 7; i++) {
        pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PAM_BASE + i, 0);
    }   
}

void q35_dram_unshadow_bios(int first_pam) {
    void (*function)(int) = (void (*)(int)) ((uintptr_t) unshadow_bios_internal + 0xfff00000);
    function(first_pam);
}

void q35_dram_smram_open() {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM,
        (pci_cfg_read_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM) | Q35_DRAM_SMRAM_OPEN) & ~Q35_DRAM_SMRAM_CLOSE);
}

void q35_dram_smram_close() {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM,
        (pci_cfg_read_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM) | Q35_DRAM_SMRAM_CLOSE) & ~Q35_DRAM_SMRAM_OPEN);
}

void q35_dram_smram_lock() {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM,
        (pci_cfg_read_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM) | Q35_DRAM_SMRAM_LOCK) & ~(Q35_DRAM_SMRAM_OPEN | Q35_DRAM_SMRAM_CLOSE));
}

void q35_dram_smram_enable() {
    pci_cfg_write_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM,
        pci_cfg_read_byte(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_SMRAM) | Q35_DRAM_SMRAM_EN);
}

void q35_dram_tolud_set(uintptr_t tolud) {
    pci_cfg_write_word(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_TOLUD, tolud & ~0b1111);
}

void q35_dram_pciexbar_enable() {
    pci_cfg_write_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR,
        pci_cfg_read_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR) | 1);
}

void q35_dram_pciexbar_disable() {
    pci_cfg_write_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR,
        pci_cfg_read_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR) & ~1);
}

void q35_dram_pciexbar_set(uint64_t pciexbar) {
    pci_cfg_write_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR,
        pci_cfg_read_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR) | (pciexbar & 0xffffffff));
    pci_cfg_write_dword(Q35_DRAM_BUS, Q35_DRAM_SLOT, Q35_DRAM_FUNCTION, Q35_DRAM_PCIEXBAR + 4, pciexbar >> 32);
}

void q35_lpc_acpi_base(uint16_t base) {
    pci_cfg_write_dword(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_PMBASE,
        pci_cfg_read_dword(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_PMBASE) | (base & 0xff00));
}

void q35_lpc_acpi_enable() {
    pci_cfg_write_byte(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_ACPI_CNTL,
        pci_cfg_read_byte(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_ACPI_CNTL) | Q35_LPC_ACPI_CNTL_EN);
}

void q35_lpc_acpi_disable() {
    pci_cfg_write_byte(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_ACPI_CNTL,
        pci_cfg_read_byte(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_ACPI_CNTL) & ~Q35_LPC_ACPI_CNTL_EN);
}

void q35_init() {
    print("lakebios: Q35: chipset found, initializing chipset specific features");
    q35_dram_unshadow_bios(1);
    // SMM setup
    q35_dram_smram_enable();
    q35_dram_smram_open();
    // Copy handler
    memcpy((void *) SMM_DEFAULT_SMBASE + SMM_SMBASE_HANDLER_OFFSET, (const void *) smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    memcpy((void *) SMM_NEW_SMBASE + SMM_SMBASE_HANDLER_OFFSET, (const void *) smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    q35_dram_smram_close();
    q35_dram_smram_lock();
    // Enable the ACPI registers and cause the APM control port (0xb2) to cause a SMI#
    q35_lpc_acpi_base(0x600);
    q35_lpc_acpi_enable();
    outd(0x600 + 0x30, ind(0x600 + 0x30) | (1 << 5) | 1);
    // Relocate SMBASE
    outb(0xb2, 0x01);
    q35_dram_tolud_set(Q35_PCIEXBAR_BASE);
    q35_dram_pciexbar_set(Q35_PCIEXBAR_BASE);
    q35_dram_pciexbar_enable();
    pci_enumerate(Q35_PCI_MMIO_BAR_BASE, Q35_PCI_IO_BAR_BASE);
    print("lakebios: Q35: finished initializing chipset specific features");
}
