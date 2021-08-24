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

#define Q35_LPC_PIRQ_A_IRQ 10
#define Q35_LPC_PIRQ_B_IRQ 10
#define Q35_LPC_PIRQ_C_IRQ 11
#define Q35_LPC_PIRQ_D_IRQ 11
#define Q35_LPC_PIRQ_E_IRQ 10
#define Q35_LPC_PIRQ_F_IRQ 10
#define Q35_LPC_PIRQ_G_IRQ 11
#define Q35_LPC_PIRQ_H_IRQ 11

#define Q35_LPC_PIRQ_A_GSI 16
#define Q35_LPC_PIRQ_B_GSI 17
#define Q35_LPC_PIRQ_C_GSI 18
#define Q35_LPC_PIRQ_D_GSI 19
#define Q35_LPC_PIRQ_E_GSI 20
#define Q35_LPC_PIRQ_F_GSI 21
#define Q35_LPC_PIRQ_G_GSI 22
#define Q35_LPC_PIRQ_H_GSI 23

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
