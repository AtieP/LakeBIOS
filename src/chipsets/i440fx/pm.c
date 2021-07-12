#include <chipsets/i440fx.h>
#include <cpu/pio.h>
#include <drivers/pci.h>

void i440fx_pm_pmba_set(uint16_t base) {
    pci_cfg_write_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_PMBA, base & ~0b111111);
}

void i440fx_pm_devactb_st(uint32_t bits) {
    pci_cfg_write_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_DEVACTB,
        pci_cfg_read_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_DEVACTB) | bits);
}

void i440fx_pm_devactb_cl(uint32_t bits) {
    pci_cfg_write_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_DEVACTB,
        pci_cfg_read_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_DEVACTB) & ~bits);
}

void i440fx_pm_smbba_set(uint16_t base) {
    pci_cfg_write_dword(I440FX_PM_BUS, I440FX_PM_SLOT, I440FX_PM_FUNCTION, I440FX_PM_SMBBA, base & ~0b1111);
}

void i440fx_pm_io_glbctl_st(uint16_t io_base, uint32_t bits) {
    outd(I440FX_PM_IO_GLBCTL(io_base), ind(I440FX_PM_IO_GLBCTL(io_base)) | bits);
}

void i440fx_pm_io_glbctl_cl(uint16_t io_base, uint32_t bits) {
    outd(I440FX_PM_IO_GLBCTL(io_base), ind(I440FX_PM_IO_GLBCTL(io_base)) & ~bits);
}
