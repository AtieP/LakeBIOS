#include <chipsets/i440fx.h>
#include <chipsets/i440fx/pm.h>
#include <drivers/pci.h>

void i440fx_pm_acpi_base(uint16_t base) {
    pci_cfg_write_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_ACPI_BASE, base);
}

void i440fx_pm_devactb_enable(uint32_t bits) {
    pci_cfg_write_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_DEVACTB,
        pci_cfg_read_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_DEVACTB) | bits);
}

void i440fx_pm_devactb_disable(uint32_t bits) {
    pci_cfg_write_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_DEVACTB,
        pci_cfg_read_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_DEVACTB) & ~bits);
}

void i440fx_pm_devresb_enable(uint32_t bits) {
    pci_cfg_write_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_DEVRESB,
        pci_cfg_read_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_DEVRESB) | bits);
}

void i440fx_pm_devresb_disable(uint32_t bits) {
    pci_cfg_write_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_DEVRESB,
        pci_cfg_read_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_DEVRESB) & ~bits);
}
