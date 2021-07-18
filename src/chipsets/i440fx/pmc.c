#include <chipsets/i440fx.h>
#include <chipsets/i440fx/pmc.h>
#include <drivers/pci.h>
#include <tools/string.h>

extern char bios_raw_start[];
extern char bios_raw_end[];

void i440fx_pmc_pam_lock(int pam) {
    pci_cfg_write_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_PAM0 + pam, 0);
}

void i440fx_pmc_pam_unlock(int pam, int copy_to_ram) {
    pci_cfg_write_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_PAM0 + pam, pam == 0 ? I440FX_PMC_PAM_UNLOCK << 4 : (I440FX_PMC_PAM_UNLOCK << 4) | I440FX_PMC_PAM_UNLOCK);
    if (copy_to_ram) {
        memcpy(bios_raw_start, 0xfff00000 + bios_raw_start, bios_raw_end - bios_raw_start);
    }
}

void i440fx_pmc_smram_open() {
    pci_cfg_write_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM, I440FX_PMC_SMRAM_OPEN | I440FX_PMC_SMRAM_EN | I440FX_PMC_SMRAM_DEFBASE);
}

void i440fx_pmc_smram_close() {
    pci_cfg_write_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM, I440FX_PMC_SMRAM_EN | I440FX_PMC_SMRAM_DEFBASE);
}

void i440fx_pmc_smram_lock() {
    pci_cfg_write_byte(I440FX_PMC_BUS, I440FX_PMC_SLOT, I440FX_PMC_FUNCTION, I440FX_PMC_SMRAM, I440FX_PMC_SMRAM_LOCK | I440FX_PMC_SMRAM_EN | I440FX_PMC_SMRAM_DEFBASE);
}
