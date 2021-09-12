#ifndef __MOTHERBOARD_QEMU_I440FX_PMC_H__
#define __MOTHERBOARD_QEMU_I440FX_PMC_H__

#define QEMU_I440FX_PMC_BUS      0
#define QEMU_I440FX_PMC_SLOT     0
#define QEMU_I440FX_PMC_FUNCTION 0

#define QEMU_I440FX_PMC_VENDOR 0x8086
#define QEMU_I440FX_PMC_DEVICE 0x1237

#define QEMU_I440FX_PMC_PAM0      0x59
#define QEMU_I440FX_PMC_PAM_EN_HI 0x30
#define QEMU_I440FX_PMC_PAM_EN_LO 0x03

#define QEMU_I440FX_PMC_SMRAM 0x72
#define QEMU_I440FX_PMC_SMRAM_OPEN     (1 << 6)
#define QEMU_I440FX_PMC_SMRAM_CLOSE    (1 << 5)
#define QEMU_I440FX_PMC_SMRAM_LOCK     (1 << 4)
#define QEMU_I440FX_PMC_SMRAM_EN       (1 << 3)
#define QEMU_I440FX_PMC_SMRAM_DBASESEG 0x02

void qemu_i440fx_pmc_pam_lock(int pam);
void qemu_i440fx_pmc_pam_unlock(int pam);

void qemu_i440fx_pmc_smram_en();
void qemu_i440fx_pmc_smram_dis();
void qemu_i440fx_pmc_smram_open();
void qemu_i440fx_pmc_smram_close();
void qemu_i440fx_pmc_smram_close_close();
void qemu_i440fx_pmc_smram_lock();

#endif
