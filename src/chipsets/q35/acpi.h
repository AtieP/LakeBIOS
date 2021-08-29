#ifndef __CHIPSETS_Q35_ACPI_H__
#define __CHIPSETS_Q35_ACPI_H__

#include <stdint.h>
#include <cpu/pio.h>

#define Q35_ACPI_BASE 0x600

#define Q35_ACPI_PM1A_EVT (Q35_ACPI_BASE + 0x00)
#define Q35_ACPI_PM1A_CNT (Q35_ACPI_BASE + 0x04)

#define Q35_ACPI_SMI_EN (Q35_ACPI_BASE + 0x30)
#define Q35_ACPI_SMI_EN_APMC (1 << 5)
#define Q35_ACPI_SMI_EN_GLB (1 << 0)

#define Q35_ACPI_SLP_EN     (1 << 13)
#define Q35_ACPI_SLP_TYP_S3 0x01
#define Q35_ACPI_SLP_TYP_S4 0x02
#define Q35_ACPI_SLP_TYP_S5 0x00

static inline void q35_acpi_smi_enable(uint32_t bits) {
    outd(Q35_ACPI_SMI_EN, ind(Q35_ACPI_SMI_EN) | bits);
}

static inline void q35_acpi_smi_disable(uint32_t bits) {
    outd(Q35_ACPI_SMI_EN, ind(Q35_ACPI_SMI_EN) &~ bits);
}

int q35_acpi_reset();
int q35_acpi_s1();
int q35_acpi_s3();
int q35_acpi_s4();
int q35_acpi_s5();

#endif
