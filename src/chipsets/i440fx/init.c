#include <chipsets/i440fx.h>
#include <chipsets/i440fx/acpi.h>
#include <chipsets/i440fx/pci2isa.h>
#include <chipsets/i440fx/pm.h>
#include <chipsets/i440fx/pmc.h>
#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/pci.h>
#include <drivers/pic.h>
#include <drivers/ps2.h>
#include <tools/string.h>
#include <tools/print.h>

static void memory_init() {
    // Avoid vulnerabilities by reloading the BIOS default data
    i440fx_pmc_pam_lock(5);
    i440fx_pmc_pam_lock(6);
    // Unlock BIOS data
    i440fx_pmc_pam_unlock(5, 1);
    i440fx_pmc_pam_unlock(6, 1);
}

static void early_acpi_init() {
    i440fx_pm_acpi_base(I440FX_ACPI_BASE);
}

static void smm_init() {
    i440fx_pmc_smram_open();
    memcpy((void *) (SMM_DEFAULT_SMBASE + SMM_SMBASE_HANDLER_OFFSET), smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    memcpy((void *) (SMM_NEW_SMBASE + SMM_SMBASE_HANDLER_OFFSET), smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    i440fx_pmc_smram_close();
    i440fx_pm_devactb_enable(I440FX_PM_DEVACTB_APMC);
    i440fx_acpi_glbctl_enable(I440FX_ACPI_GLBCTL_SMI);
    outb(0xb2, 0x01);
}

static void irqs_init() {
    pic_init(8, 0xa0);
    // PIRQs
    for (int i = 0; i < 4; i++) {
        i440fx_pci2isa_pirq_disable(i);
    }
    i440fx_pci2isa_pirq_route(0, I440FX_PCI2ISA_PIRQ_A_IRQ);
    pic_set_level(I440FX_PCI2ISA_PIRQ_A_IRQ);
    i440fx_pci2isa_pirq_route(1, I440FX_PCI2ISA_PIRQ_B_IRQ);
    pic_set_level(I440FX_PCI2ISA_PIRQ_B_IRQ);
    i440fx_pci2isa_pirq_route(2, I440FX_PCI2ISA_PIRQ_C_IRQ);
    pic_set_level(I440FX_PCI2ISA_PIRQ_C_IRQ);
    i440fx_pci2isa_pirq_route(3, I440FX_PCI2ISA_PIRQ_D_IRQ);
    pic_set_level(I440FX_PCI2ISA_PIRQ_D_IRQ);
    for (int i = 0; i < 4; i++) {
        i440fx_pci2isa_pirq_enable(i);
    }
}

static void isa_init() {
    if (ps2_init() != 0) {
        print("lakebios: could not initialize PS/2 controller. Halting.");
    }
}

static void pci_init() {
    pci_enumerate(
        I440FX_PCI_MMIO_BASE, I440FX_PCI_IO_BASE,
        I440FX_PCI2ISA_PIRQ_A_IRQ, I440FX_PCI2ISA_PIRQ_B_IRQ, I440FX_PCI2ISA_PIRQ_C_IRQ, I440FX_PCI2ISA_PIRQ_D_IRQ);
}

void i440fx_init() {
    memory_init();
    early_acpi_init();
    smm_init();
    irqs_init();
    isa_init();
    pci_init();
}