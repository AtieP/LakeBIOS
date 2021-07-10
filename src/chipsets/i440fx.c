#include <chipsets/i440fx.h>
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
        pam = pci_cfg_read_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_PAM_BASE + i);
        pam |= 0b11 << 4;
        if (i != 0) {
            pam |= 0b11;
        }
        pci_cfg_write_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_PAM_BASE + i, pam);
    }
    // Copy now the BIOS into ram
    memcpy(bios_raw_start, bios_raw_start + 0xfff00000, bios_raw_end - bios_raw_start - (first_pam * 0x10000));
}

void i440fx_pmc_shadow_bios(int first_pam) {
    for (int i = first_pam; i < 7; i++) {
        pci_cfg_write_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_PAM_BASE + i, 0);
    }   
}

void i440fx_pmc_unshadow_bios(int first_pam) {
    void (*function)(int) = (void (*)(int)) ((uintptr_t) unshadow_bios_internal + 0xfff00000);
    function(first_pam);
}

void i440fx_pmc_smram_open() {
    pci_cfg_write_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM,
        (pci_cfg_read_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM) | I440FX_PMC_SMRAM_OPEN) & ~I440FX_PMC_SMRAM_CLOSE);
}

void i440fx_pmc_smram_close() {
    pci_cfg_write_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM,
        (pci_cfg_read_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM) | I440FX_PMC_SMRAM_CLOSE) & ~I440FX_PMC_SMRAM_OPEN);
}

void i440fx_pmc_smram_lock() {
    pci_cfg_write_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM,
        (pci_cfg_read_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM) | I440FX_PMC_SMRAM_LOCK) & ~(I440FX_PMC_SMRAM_OPEN | I440FX_PMC_SMRAM_CLOSE));
}

void i440fx_pmc_smram_enable() {
    pci_cfg_write_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM,
        pci_cfg_read_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM) | I440FX_PMC_SMRAM_EN);
}

void i440fx_pmc_smram_base(uint8_t base) {
    pci_cfg_write_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM,
        pci_cfg_read_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM) | base);
}

void i440fx_init() {
    print("lakebios: I440FX: chipset found, initializing chipset specific features");
    i440fx_pmc_unshadow_bios(1);
    // SMM setup
    i440fx_pmc_smram_enable();
    i440fx_pmc_smram_base(I440FX_PMC_SMRAM_DBASESEG_DEF);
    i440fx_pmc_smram_open();
    // Copy handler
    memcpy((void *) SMM_DEFAULT_SMBASE + SMM_SMBASE_HANDLER_OFFSET, (const void *) smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    memcpy((void *) SMM_NEW_SMBASE + SMM_SMBASE_HANDLER_OFFSET, (const void *) smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    i440fx_pmc_smram_close();
    i440fx_pmc_smram_lock();
    // Generate a SMI# when writing to the APM control register
    pci_cfg_write_dword(I440FX_PIIX4_BUS, I440FX_PIIX4_SLOT, I440FX_PIIX4_FUNCTION, 0x58, (1 << 25));
    // Generate SMI#s
    outd(0x600 + 0x28, ind(0x600 + 0x28) | 1);
    // Relocate SMBASE
    outb(0xb2, 0x01);
    pci_enumerate(I440FX_PCI_MMIO_BASE, I440FX_PCI_IO_BASE);
    print("lakebios: I440FX: finished initializing chipset specific features");
}