#ifndef __CHIPSETS_Q35_LPC_H__
#define __CHIPSETS_Q35_LPC_H__

#include <stdint.h>

#define Q35_LPC_BUS 0
#define Q35_LPC_SLOT 31
#define Q35_LPC_FUNCTION 0

#define Q35_LPC_ACPI_BASE 0x40
#define Q35_LPC_ACPI_CNTL 0x44
#define Q35_LPC_ACPI_CNTL_EN (1 << 7)

#define Q35_LPC_PIRQ_A 0x60
#define Q35_LPC_PIRQ_E 0x68
#define Q35_LPC_PIRQ_EN (1 << 7)

#define Q35_LPC_ID 0x80 // I/O decode (used for selecting the serial port and floppy ranges)

#define Q35_LPC_IF_ENABLES 0x82
#define Q35_LPC_IF_KBC (1 << 10)
#define Q35_LPC_IF_FDD (1 << 3)
#define Q35_LPC_IF_LPT (1 << 2)
#define Q35_LPC_IF_COMB (1 << 1)
#define Q35_LPC_IF_COMA (1 << 0)

void q35_lpc_acpi_base(uint16_t base);
void q35_lpc_acpi_enable();
void q35_lpc_acpi_disable();
void q35_lpc_acpi_sci_irq(uint8_t irq);

void q35_lpc_pirq_enable(int pirq);
void q35_lpc_pirq_disable(int pirq);
void q35_lpc_pirq_route(int pirq, uint8_t irq);

void q35_lpc_if_enable(uint16_t bits);
void q35_lpc_if_disable(uint16_t bits);

#endif
