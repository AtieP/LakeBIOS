#ifndef __CHIPSETS_I440FX_ACPI_H__
#define __CHIPSETS_I440FX_ACPI_H__

#include <stdint.h>
#include <cpu/pio.h>

#define I440FX_ACPI_BASE 0x600

#define I440FX_ACPI_PM1A_EVT (I440FX_ACPI_BASE + 0x00)
#define I440FX_ACPI_PM1A_CNT (I440FX_ACPI_BASE + 0x04)

#define I440FX_ACPI_GLBCTL (I440FX_ACPI_BASE + 0x28)
#define I440FX_ACPI_GLBCTL_SMI (1 << 0)

#define I440FX_ACPI_SLP_EN     (1 << 13)
#define I440FX_ACPI_SLP_TYP_S3 0x01
#define I440FX_ACPI_SLP_TYP_S4 0x02
#define I440FX_ACPI_SLP_TYP_S5 0x00

static inline void i440fx_acpi_glbctl_enable(uint32_t bits) {
    outd(I440FX_ACPI_GLBCTL, ind(I440FX_ACPI_GLBCTL) | bits);
}

static inline void i440fx_acpi_glbctl_disable(uint32_t bits) {
    outd(I440FX_ACPI_GLBCTL, ind(I440FX_ACPI_GLBCTL) & ~bits);
}

int i440fx_acpi_reset();
int i440fx_acpi_s3();
int i440fx_acpi_s4();
int i440fx_acpi_s5();

#endif
