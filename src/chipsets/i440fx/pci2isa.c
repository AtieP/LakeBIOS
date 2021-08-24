#include <chipsets/i440fx.h>
#include <chipsets/i440fx/pci2isa.h>
#include <drivers/pci.h>

void i440fx_pci2isa_pirq_enable(int pirq) {
    pci_cfg_write_byte(I440FX_PCI2ISA_BUS, I440FX_PCI2ISA_SLOT, I440FX_PCI2ISA_FUNCTION, I440FX_PCI2ISA_PIRQ_A + pirq,
        pci_cfg_read_byte(I440FX_PCI2ISA_BUS, I440FX_PCI2ISA_SLOT, I440FX_PCI2ISA_FUNCTION, I440FX_PCI2ISA_PIRQ_A + pirq)
        | I440FX_PCI2ISA_PIRQ_EN
    );
}

void i440fx_pci2isa_pirq_disable(int pirq) {
    pci_cfg_write_byte(I440FX_PCI2ISA_BUS, I440FX_PCI2ISA_SLOT, I440FX_PCI2ISA_FUNCTION, I440FX_PCI2ISA_PIRQ_A + pirq,
        pci_cfg_read_byte(I440FX_PCI2ISA_BUS, I440FX_PCI2ISA_SLOT, I440FX_PCI2ISA_FUNCTION, I440FX_PCI2ISA_PIRQ_A + pirq)
        & ~I440FX_PCI2ISA_PIRQ_EN
    );
}

void i440fx_pci2isa_pirq_route(int pirq, uint8_t irq) {
    pci_cfg_write_byte(I440FX_PCI2ISA_BUS, I440FX_PCI2ISA_SLOT, I440FX_PCI2ISA_FUNCTION, I440FX_PCI2ISA_PIRQ_A + pirq,
        (pci_cfg_read_byte(I440FX_PCI2ISA_BUS, I440FX_PCI2ISA_SLOT, I440FX_PCI2ISA_FUNCTION, I440FX_PCI2ISA_PIRQ_A + pirq)
        & ~0x0f) | irq
    );
}
