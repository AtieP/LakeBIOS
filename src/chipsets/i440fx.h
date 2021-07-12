#ifndef __CHIPSETS_I440FX_H__
#define __CHIPSETS_I440FX_H__

#include <stdint.h>

#define I440FX_PCI_BAR_MMIO_BASE 0xc0000000
#define I440FX_PCI_BAR_IO_BASE 0xc000

// PMC

#define I440FX_PMC_BUS 0
#define I440FX_PMC_SLOT 0
#define I440FX_PMC_FUNCTION 0

#define I440FX_PMC_VENDOR 0x8086
#define I440FX_PMC_DEVICE 0x1237

#define I440FX_PMC_PAM_BASE 0x59
#define I440FX_PMC_PAM_UNLOCK 0b11

#define I440FX_PMC_FDHC 0x68
#define I440FX_PMC_FDHC_EXTMEM (1 << 7)

#define I440FX_PMC_SMRAM 0x72
#define I440FX_PMC_SMRAM_OPEN (1 << 6)
#define I440FX_PMC_SMRAM_CLOSE (1 << 5)
#define I440FX_PMC_SMRAM_LOCK (1 << 4)
#define I440FX_PMC_SMRAM_ENABLE (1 << 3)
#define I440FX_PMC_SMRAM_DBASESEG 0b010

void i440fx_pmc_pam_lock(int pam);
void i440fx_pmc_pam_unlock(int pam, int copy_to_ram);

void i440fx_pmc_hen_extmem();
void i440fx_pmc_hdis_extmem();

void i440fx_pmc_smram_open();
void i440fx_pmc_smram_close();
void i440fx_pmc_smram_lock();

// PCI to ISA bridge

#define I440FX_PCI2ISA_BUS 0
#define I440FX_PCI2ISA_SLOT 1
#define I440FX_PCI2ISA_FUNCTION 0

// IDE controller

#define I440FX_IDE_BUS 0
#define I440FX_IDE_SLOT 1
#define I440FX_IDE_FUNCTION 1

// Power management

#define I440FX_PM_BUS 0
#define I440FX_PM_SLOT 1
#define I440FX_PM_FUNCTION 3

#define I440FX_PM_PMBA 0x40

#define I440FX_PM_DEVACTB 0x58
#define I440FX_PM_DEVACTB_APMC_EN (1 << 25)

#define I440FX_PM_SMBBA 0x90

#define I440FX_PM_IO_GLBCTL(base) (base + 0x28)
#define I440FX_PM_IO_GLBCTL_SMI_EN (1 << 0)

void i440fx_pm_pmba_set(uint16_t base);
void i440fx_pm_devactb_st(uint32_t bits);
void i440fx_pm_devactb_cl(uint32_t bits);
void i440fx_pm_smbba_set(uint16_t base);

void i440fx_pm_io_glbctl_st(uint16_t io_base, uint32_t bits);
void i440fx_pm_io_glbctl_cl(uint16_t io_base, uint32_t bits);

// Init
void i440fx_init();

#endif
