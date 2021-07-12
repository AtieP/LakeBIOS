#include <chipsets/i440fx.h>
#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/pci.h>
#include <tools/string.h>

void i440fx_init() {
    // Unshadow BIOS data
    i440fx_pmc_pam_unlock(5, 1); // 0xe0000-0xe7fff
    i440fx_pmc_pam_unlock(6, 1); // 0xe8000-0xeffff
    // ACPI setup
    i440fx_pm_pmba_set(0x600);
    i440fx_pm_devactb_st(I440FX_PM_DEVACTB_APMC_EN);
    i440fx_pm_io_glbctl_st(0x600, I440FX_PM_IO_GLBCTL_SMI_EN);
    // SMRAM setup
    i440fx_pmc_smram_open();
    memcpy((void *) (SMM_DEFAULT_SMBASE + SMM_SMBASE_HANDLER_OFFSET), smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    memcpy((void *) (SMM_NEW_SMBASE + SMM_SMBASE_HANDLER_OFFSET), smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    i440fx_pmc_smram_lock(); // Close is implicit
    outb(0xb2, 0x01);
    // PCI setup
    pci_enumerate(I440FX_PCI_BAR_MMIO_BASE, I440FX_PCI_BAR_IO_BASE);
}
