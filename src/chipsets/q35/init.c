#include <chipsets/q35.h>
#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/pci.h>
#include <tools/string.h>

void q35_init() {
    // Unlock BIOS data
    q35_dram_pam_unlock(5, 1); // 0xe0000-0xe7fff
    q35_dram_pam_unlock(6, 1); // 0xe8000-0xeffff
    // ACPI setup
    q35_lpc_pmbase(0x600);
    q35_lpc_acpi_sci_irq(9);
    q35_lpc_acpi_enable();
    // SMRAM setup
    q35_dram_smram_open();
    memcpy((void *) (SMM_DEFAULT_SMBASE + SMM_SMBASE_HANDLER_OFFSET), smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    memcpy((void *) (SMM_NEW_SMBASE + SMM_SMBASE_HANDLER_OFFSET), smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    q35_dram_smram_lock(); // Close is implicit
    outd(0x600 + 0x30, ind(0x600 + 0x30) | (1 << 5) | 1);
    outb(0xb2, 0x01);
    // PCI setup
    q35_dram_tolud(Q35_PCIEXBAR);
    q35_dram_pciexbar_set(Q35_PCIEXBAR);
    q35_dram_pciexbar_enable();
    pci_enumerate(Q35_PCI_BAR_MMIO_BASE, Q35_PCI_BAR_IO_BASE);
}