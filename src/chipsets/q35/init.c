#include <chipsets/q35.h>
#include <chipsets/q35/acpi.h>
#include <chipsets/q35/dram.h>
#include <chipsets/q35/lpc.h>
#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/pci.h>
#include <drivers/pic.h>
#include <drivers/ps2.h>
#include <tools/print.h>
#include <tools/string.h>

static void memory_init() {
    // Shadow and unshadow BIOS data, to avoid exploits
    q35_dram_pam_lock(5);
    q35_dram_pam_lock(6);
    q35_dram_pam_unlock(5, 1);
    q35_dram_pam_unlock(6, 1);
}

static void early_acpi_init() {
    // Enable ACPI registers
    q35_lpc_acpi_base(Q35_ACPI_BASE);
    q35_lpc_acpi_enable();
}

static void smm_init() {
    // SMRAM setup
    q35_dram_smram_open();
    memcpy((void *) (SMM_DEFAULT_SMBASE + SMM_SMBASE_HANDLER_OFFSET), smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    memcpy((void *) (SMM_NEW_SMBASE + SMM_SMBASE_HANDLER_OFFSET), smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    q35_dram_smram_close();
    q35_acpi_smi_enable(Q35_ACPI_SMI_EN_APMC | Q35_ACPI_SMI_EN_GLB);
    outb(0xb2, 0x01);
    memset((void *) SMM_DEFAULT_SMBASE, 0, 0x20000);
}

static void irqs_init() {
    // PIRQs
    for (int i = 1; i < 8; i++) {
        q35_lpc_pirq_disable(i);
    }
    q35_lpc_pirq_route(0, 10);
    q35_lpc_pirq_route(1, 11);
    q35_lpc_pirq_route(2, 10);
    q35_lpc_pirq_route(3, 11);
    q35_lpc_pirq_enable(0);
    q35_lpc_pirq_enable(1);
    q35_lpc_pirq_enable(2);
    q35_lpc_pirq_enable(3);
    // SCI
    q35_lpc_acpi_sci_irq(9);
    // Others
    pic_init(8, 0xa0);
}

static void isa_init() {
    // PS/2 init
    q35_lpc_if_enable(Q35_LPC_IF_KBC);
    if (ps2_init() != 0) {
        print("lakebios: could not initialize PS/2. Halting.");
        for (;;) {}
    }
    // Serial and parallel ports, also floppy init (but WIP)
    q35_lpc_if_enable(Q35_LPC_IF_COMA | Q35_LPC_IF_COMB | Q35_LPC_IF_LPT | Q35_LPC_IF_FDD);
    pci_cfg_write_word(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_ID,
          (0 << 12) /* FDD range: 0x3f0-0x3f5, 0x3f7 */
        | (0 << 8)  /* LPT range: 0x378-0x3f7, 0x778-0x77f */
        | (1 << 4)  /* COMB range: 0x2f8-0x2ff */
        | (0 << 0)  /* COMA range: 0x3f8-0x3ff */
    );
}

static void pci_init() {
    pci_enumerate(Q35_PCI_MMIO_BASE, Q35_PCI_IO_BASE, 10, 11, 10, 11);
}

void q35_init() {
    memory_init();
    early_acpi_init();
    smm_init();
    irqs_init();
    isa_init();
    pci_init();
}
