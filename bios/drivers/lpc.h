#ifndef __DRIVERS_LPC_H__
#define __DRIVERS_LPC_H__

#include <stdint.h>

#define LPC_BUS 0
#define LPC_SLOT 31
#define LPC_FUNCTION 0

#define LPC_PMBASE 0x40
#define LPC_ACPI_CNTL 0x44

#define LPC_ACPI_CNTL_EN (1 << 7)

void lpc_set_acpi_base(uint16_t base);
void lpc_enable_acpi();
void lpc_disable_acpi();

#endif
