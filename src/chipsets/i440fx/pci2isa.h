#ifndef __CHIPSETS_I440FX_PCI2ISA_H__
#define __CHIPSETS_I440FX_PCI2ISA_H__

#include <stdint.h>

#define I440FX_PCI2ISA_BUS 0
#define I440FX_PCI2ISA_SLOT 1
#define I440FX_PCI2ISA_FUNCTION 0

#define I440FX_PCI2ISA_PIRQ_A 0x60
#define I440FX_PCI2ISA_PIRQ_EN (1 << 7)

// Defines for what IRQs/GSIs the PIRQs match.
// The IRQs can be changed, but the GSIs are
// hardcoded as per spec
#define I440FX_PCI2ISA_PIRQ_A_IRQ 10
#define I440FX_PCI2ISA_PIRQ_B_IRQ 11
#define I440FX_PCI2ISA_PIRQ_C_IRQ 10
#define I440FX_PCI2ISA_PIRQ_D_IRQ 11

// Not sure about the GSIs though.
// #define I440FX_PCI2ISA_PIRQ_A_GSI 18
// #define I440FX_PCI2ISA_PIRQ_B_GSI 19
// #define I440FX_PCI2ISA_PIRQ_C_GSI 20
// #define I440FX_PCI2ISA_PIRQ_D_GSI 21

void i440fx_pci2isa_pirq_enable(int pirq);
void i440fx_pci2isa_pirq_disable(int pirq);
void i440fx_pci2isa_pirq_route(int pirq, uint8_t irq);

#endif