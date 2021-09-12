#ifndef __MOTHERBOARD_QEMU_PIIX3_PCI_ISA_H__
#define __MOTHERBOARD_QEMU_PIIX3_PCI_ISA_H__

#include <stdint.h>

#define QEMU_PIIX3_PCI_ISA_BUS      0
#define QEMU_PIIX3_PCI_ISA_SLOT     1
#define QEMU_PIIX3_PCI_ISA_FUNCTION 0

#define QEMU_PIIX3_PCI_ISA_PIRQA     0x60
#define QEMU_PIIX3_PCI_ISA_PIRQ_DIS  (1 << 7)
#define QEMU_PIIX3_PCI_ISA_PIRQ_MASK 0x0f

extern const uint8_t qemu_piix3_pci_isa_pirq_map[];

void qemu_piix3_pci_isa_pirq_route(int pirq, int irq);
void qemu_piix3_pci_isa_pirq_en(int pirq);
void qemu_piix3_pci_isa_pirq_dis(int pirq);

#endif
