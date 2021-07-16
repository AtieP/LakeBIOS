#ifndef __CHIPSETS_I440FX_PM_H__
#define __CHIPSETS_I440FX_PM_H__

#include <stdint.h>

#define I440FX_PM_BUS 0
#define I440FX_PM_SLOT 1
#define I440FX_PM_FUNCTION 3

#define I440FX_PM_ACPI_BASE 0x40

#define I440FX_PM_DEVACTB 0x58
#define I440FX_PM_DEVACTB_APMC (1 << 25)

#define I440FX_PM_DEVRESB 0x60

void i440fx_pm_acpi_base(uint16_t base);

void i440fx_pm_devactb_enable(uint32_t bits);
void i440fx_pm_devactb_disable(uint32_t bits);

void i440fx_pm_devresb_enable(uint32_t bits);
void i440fx_pm_devresb_disable(uint32_t bits);

#endif
