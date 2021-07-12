#include <chipsets/q35.h>
#include <drivers/pci.h>

void q35_lpc_pmbase(uint16_t pmbase) {
    pci_cfg_write_dword(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_PMBASE,
        pci_cfg_read_dword(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_PMBASE) | (pmbase & 0xff00));
}

void q35_lpc_acpi_enable() {
    pci_cfg_write_byte(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_ACPI_CNTL,
        pci_cfg_read_byte(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_ACPI_CNTL_EN) | Q35_LPC_ACPI_CNTL_EN);
}

void q35_lpc_acpi_disable() {
    pci_cfg_write_byte(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_ACPI_CNTL,
        pci_cfg_read_byte(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_ACPI_CNTL_EN) & ~Q35_LPC_ACPI_CNTL_EN);
}

void q35_lpc_acpi_sci_irq(int irq) {
    if (irq == 20) {
        irq = 0b100;
    } else if (irq == 21) {
        irq = 0b101;
    } else {
        irq = irq - 9;
    }
    pci_cfg_write_byte(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_ACPI_CNTL,
        pci_cfg_read_byte(Q35_LPC_BUS, Q35_LPC_SLOT, Q35_LPC_FUNCTION, Q35_LPC_ACPI_CNTL_EN) | irq);
}
