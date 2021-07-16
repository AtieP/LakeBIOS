#ifndef __CHIPSETS_I440FX_ACPI_H__
#define __CHIPSETS_I440FX_ACPI_H__

#include <stdint.h>
#include <cpu/pio.h>

#define I440FX_ACPI_BASE 0x600

#define I440FX_ACPI_PM1A_EVT (I440FX_ACPI_BASE + 0x00)
#define I440FX_ACPI_PM1A_CNT (I440FX_ACPI_BASE + 0x04)

#define I440FX_ACPI_GLBCTL (I440FX_ACPI_BASE + 0x28)
#define I440FX_ACPI_GLBCTL_SMI (1 << 0)

static inline void i440fx_acpi_glbctl_enable(uint32_t bits) {
    outd(I440FX_ACPI_GLBCTL, ind(I440FX_ACPI_GLBCTL) | bits);
}

static inline void i440fx_acpi_glbctl_disable(uint32_t bits) {
    outd(I440FX_ACPI_GLBCTL, ind(I440FX_ACPI_GLBCTL) & ~bits);
}

#endif
