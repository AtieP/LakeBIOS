#ifndef __CHIPSETS_I440FX_H__
#define __CHIPSETS_I440FX_H__

#include <stdint.h>

#define I440FX_PCI_MMIO_BASE 0xc0000000
#define I440FX_PCI_IO_BASE 0xc000

#define I440FX_PMC_BUS 0
#define I440FX_PMC_SLOT 0
#define I440FX_PMC_FUNCTION 0
#define I440FX_PMC_VENDOR 0x8086
#define I440FX_PMC_DEVICE 0x1237
#define I440FX_PMC_PAM_BASE 0x59
#define I440FX_PMC_SMRAM 0x72

#define I440FX_PMC_SMRAM_OPEN (1 << 6)
#define I440FX_PMC_SMRAM_CLOSE (1 << 5)
#define I440FX_PMC_SMRAM_LOCK (1 << 4)
#define I440FX_PMC_SMRAM_EN (1 << 3)
#define I440FX_PMC_SMRAM_DBASESEG_DEF 0b010

#define I440FX_PIIX4_BUS 0
#define I440FX_PIIX4_SLOT 1
#define I440FX_PIIX4_FUNCTION 3

void i440fx_pmc_shadow_bios();
void i440fx_pmc_unshadow_bios();
void i440fx_pmc_smram_open();
void i440fx_pmc_smram_close();
void i440fx_pmc_smram_lock();
void i440fx_pmc_smram_enable();
void i440fx_pmc_smram_base(uint8_t base);
void i440fx_init();

#endif
