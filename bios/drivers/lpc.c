#include <drivers/lpc.h>
#include <drivers/pci.h>

// Sets the ACPI base register
void lpc_set_acpi_base(uint16_t base) {
    pci_cfg_write_dword(LPC_BUS, LPC_SLOT, LPC_FUNCTION, LPC_PMBASE, ((uint32_t) base & 0xff00) | 1);
}

// Enables the ACPI base register
void lpc_enable_acpi() {
    pci_cfg_write_byte(LPC_BUS, LPC_SLOT, LPC_FUNCTION, LPC_ACPI_CNTL, LPC_ACPI_CNTL_EN);
}

// Disables the ACPI base register
void lpc_disable_acpi() {
    pci_cfg_write_byte(LPC_BUS, LPC_SLOT, LPC_FUNCTION, LPC_ACPI_CNTL, 0);
}
