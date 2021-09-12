#ifndef __MOTHERBOARD_QEMU_PIIX4_PM_H__
#define __MOTHERBOARD_QEMU_PIIX4_PM_H__

#include <stdint.h>

#define QEMU_PIIX4_PM_BUS      0
#define QEMU_PIIX4_PM_SLOT     1
#define QEMU_PIIX4_PM_FUNCTION 3

#define QEMU_PIIX4_PM_PMBA 0x40
#define QEMU_PIIX4_PM_PMBA_MASK ~0x3f

#define QEMU_PIIX4_PM_DEVACTA 0x58
#define QEMU_PIIX4_PM_APMC_EN (1 << 25)

#define QEMU_PIIX4_PM_PMREGMISC        0x80
#define QEMU_PIIX4_PM_PMREGMISC_PMIOSE 0x01

#define QEMU_PIIX4_PM_SMBBA       0x90
#define QEMU_PIIX4_PM_SMBBA_MASK ~0x0f

#define QEMU_PIIX4_PM_SMBHSTCFG            0xd2
#define QEMU_PIIX4_PM_SMBHSTCFG_SMB_HST_EN 0x01
#define QEMU_PIIX4_PM_SMBHSTCFG_IRQ_MASK   ~(0x07 << 1)
#define QEMU_PIIX4_PM_SMBHSTCFG_IRQ_SMI    (0x00 << 1)
#define QEMU_PIIX4_PM_SMBHSTCFG_IRQ_9      (0x04 << 1)

void qemu_piix4_pm_pmba_set(uint16_t pmba);

void qemu_piix4_pm_devacta_set(uint32_t bits);
void qemu_piix4_pm_devacta_clear(uint32_t bits);

void qemu_piix4_pm_pmregmisc_pmba_en();
void qemu_piix4_pm_pmregmisc_pmba_dis();

void qemu_piix4_pm_smbba_set(uint16_t smbba);

void qemu_piix4_pm_smbhstcfg_smbba_en();
void qemu_piix4_pm_smbhstcfg_smbba_dis();
void qemu_piix4_pm_smbhstcfg_smbba_irq(uint8_t irq);

#endif
