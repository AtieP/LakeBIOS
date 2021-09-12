#ifndef __MOTHERBOARD_QEMU_PIIX4_ACPI_H__
#define __MOTHERBOARD_QEMU_PIIX4_ACPI_H__

#include <cpu/pio.h>

#define QEMU_PIIX4_ACPI_PMBASE 0x600

#define QEMU_PIIX4_ACPI_PM1A_CNT 0x04
#define QEMU_PIIX4_ACPI_PM1A_CNT_SLP_EN (1 << 13)
#define QEMU_PIIX4_ACPI_PM1A_CNT_SLP_TYP_S3 (1 << 10)
#define QEMU_PIIX4_ACPI_PM1A_CNT_SLP_TYP_S4 (2 << 10)
#define QEMU_PIIX4_ACPI_PM1A_CNT_SLP_TYP_S5 (0 << 10)

static inline int qemu_piix4_acpi_pm1a_cnt_slp(int slp_typ) {
    if (slp_typ == 3) {
        outw(QEMU_PIIX4_ACPI_PMBASE + QEMU_PIIX4_ACPI_PM1A_CNT, QEMU_PIIX4_ACPI_PM1A_CNT_SLP_EN | QEMU_PIIX4_ACPI_PM1A_CNT_SLP_TYP_S3);
        return 0;
    } else if (slp_typ == 4) {
        outw(QEMU_PIIX4_ACPI_PMBASE + QEMU_PIIX4_ACPI_PM1A_CNT, QEMU_PIIX4_ACPI_PM1A_CNT_SLP_EN | QEMU_PIIX4_ACPI_PM1A_CNT_SLP_TYP_S4);
        return 0;
    } else if (slp_typ == 5) {
        outw(QEMU_PIIX4_ACPI_PMBASE + QEMU_PIIX4_ACPI_PM1A_CNT, QEMU_PIIX4_ACPI_PM1A_CNT_SLP_EN | QEMU_PIIX4_ACPI_PM1A_CNT_SLP_TYP_S5);
        return 0;
    } else {
        return -1;
    }
}

#endif
