#ifndef __MOTHERBOARD_QEMU_ICH9_ACPI_H__
#define __MOTHERBOARD_QEMU_ICH9_ACPI_H__

#include <stdint.h>
#include <cpu/pio.h>

#define QEMU_ICH9_ACPI_PMBASE 0x600

#define QEMU_ICH9_ACPI_PM1A_CNT            0x04
#define QEMU_ICH9_ACPI_PM1A_CNT_SLP_EN     (1 << 13)
#define QEMU_ICH9_ACPI_PM1A_CNT_SLP_TYP_S3 (1 << 10)
#define QEMU_ICH9_ACPI_PM1A_CNT_SLP_TYP_S4 (2 << 10)
#define QEMU_ICH9_ACPI_PM1A_CNT_SLP_TYP_S5 (0 << 10)

#define QEMU_ICH9_ACPI_SMI_EN      0x30
#define QEMU_ICH9_ACPI_SMI_APMC_EN (1 << 5)
#define QEMU_ICH9_ACPI_SMI_GLB     (1 << 0)

static inline int qemu_ich9_acpi_pm1a_cnt_slp(int slp_typ) {
    if (slp_typ == 3) {
        outw(QEMU_ICH9_ACPI_PMBASE + QEMU_ICH9_ACPI_PM1A_CNT, QEMU_ICH9_ACPI_PM1A_CNT_SLP_EN | QEMU_ICH9_ACPI_PM1A_CNT_SLP_TYP_S3);
        return 0;
    } else if (slp_typ == 4) {
        outw(QEMU_ICH9_ACPI_PMBASE + QEMU_ICH9_ACPI_PM1A_CNT, QEMU_ICH9_ACPI_PM1A_CNT_SLP_EN | QEMU_ICH9_ACPI_PM1A_CNT_SLP_TYP_S4);
        return 0;
    } else if (slp_typ == 5) {
        outw(QEMU_ICH9_ACPI_PMBASE + QEMU_ICH9_ACPI_PM1A_CNT, QEMU_ICH9_ACPI_PM1A_CNT_SLP_EN | QEMU_ICH9_ACPI_PM1A_CNT_SLP_TYP_S5);
        return 0;
    } else {
        return -1;
    }
}

static inline void qemu_ich9_acpi_smi_en_set(uint32_t bits) {
    outd(QEMU_ICH9_ACPI_PMBASE + QEMU_ICH9_ACPI_SMI_EN, ind(QEMU_ICH9_ACPI_PMBASE + QEMU_ICH9_ACPI_SMI_EN) | bits);
}

static inline void qemu_ich9_acpi_smi_en_clear(uint32_t bits) {
    outd(QEMU_ICH9_ACPI_PMBASE + QEMU_ICH9_ACPI_SMI_EN, ind(QEMU_ICH9_ACPI_PMBASE + QEMU_ICH9_ACPI_SMI_EN) & ~bits);
}

#endif
