#include <drivers/bus/pci.h>
#include <motherboard/qemu/piix3/pci_isa.h>

// PIRQ map
const uint8_t qemu_piix3_pci_isa_pirq_map[] = {
    10, 10, 11, 11
};

static uint8_t pci_isa_read_byte(uint8_t offset) {
    return pci_cfg_read_byte(QEMU_PIIX3_PCI_ISA_BUS, QEMU_PIIX3_PCI_ISA_SLOT, QEMU_PIIX3_PCI_ISA_FUNCTION, offset);
}

static void pci_isa_write_byte(uint8_t offset, uint8_t value) {
    pci_cfg_write_byte(QEMU_PIIX3_PCI_ISA_BUS, QEMU_PIIX3_PCI_ISA_SLOT, QEMU_PIIX3_PCI_ISA_FUNCTION, offset, value);
}

/* PCI IRQ routing */

void qemu_piix3_pci_isa_pirq_route(int pirq, int irq) {
    pci_isa_write_byte(QEMU_PIIX3_PCI_ISA_PIRQA + pirq, (pci_isa_read_byte(QEMU_PIIX3_PCI_ISA_PIRQA + pirq) & ~QEMU_PIIX3_PCI_ISA_PIRQ_MASK) | (irq & QEMU_PIIX3_PCI_ISA_PIRQ_MASK));
}

void qemu_piix3_pci_isa_pirq_en(int pirq) {
    pci_isa_write_byte(QEMU_PIIX3_PCI_ISA_PIRQA + pirq, pci_isa_read_byte(QEMU_PIIX3_PCI_ISA_PIRQA + pirq) & ~QEMU_PIIX3_PCI_ISA_PIRQ_DIS);
}

void qemu_piix3_pci_isa_pirq_dis(int pirq) {
    pci_isa_write_byte(QEMU_PIIX3_PCI_ISA_PIRQA + pirq, pci_isa_read_byte(QEMU_PIIX3_PCI_ISA_PIRQA + pirq) | QEMU_PIIX3_PCI_ISA_PIRQ_DIS);
}
