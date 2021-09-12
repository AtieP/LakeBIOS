#include <drivers/bus/pci.h>
#include <motherboard/qemu/ich9/lpc.h>

// PIRQ map
const uint8_t qemu_ich9_lpc_pirq_map[] = {
    10, 10, 11, 11,
    10, 10, 11, 11
};

static inline uint8_t lpc_read_byte(uint8_t offset) {
    return pci_cfg_read_byte(QEMU_ICH9_LPC_BUS, QEMU_ICH9_LPC_SLOT, QEMU_ICH9_LPC_FUNCTION, offset);
}

static inline uint32_t lpc_read_dword(uint8_t offset) {
    return pci_cfg_read_dword(QEMU_ICH9_LPC_BUS, QEMU_ICH9_LPC_SLOT, QEMU_ICH9_LPC_FUNCTION, offset);
}

static inline void lpc_write_byte(uint8_t offset, uint8_t value) {
    pci_cfg_write_byte(QEMU_ICH9_LPC_BUS, QEMU_ICH9_LPC_SLOT, QEMU_ICH9_LPC_FUNCTION, offset, value);
}

static inline void lpc_write_dword(uint8_t offset, uint32_t value) {
    pci_cfg_write_dword(QEMU_ICH9_LPC_BUS, QEMU_ICH9_LPC_SLOT, QEMU_ICH9_LPC_FUNCTION, offset, value);
}

/* Power Management Base */

void qemu_ich9_lpc_pmbase(uint16_t pmbase) {
    lpc_write_dword(QEMU_ICH9_LPC_PMBASE, pmbase & QEMU_ICH9_LPC_PMBASE_MASK);
}

/* ACPI Control */

void qemu_ich9_lpc_acpi_cntl_pmbase_en() {
    lpc_write_byte(QEMU_ICH9_LPC_ACPI_CNTL, lpc_read_byte(QEMU_ICH9_LPC_ACPI_CNTL) | QEMU_ICH9_LPC_ACPI_CNTL_EN);
}

void qemu_ich9_lpc_acpi_cntl_pmbase_dis() {
    lpc_write_byte(QEMU_ICH9_LPC_ACPI_CNTL, lpc_read_byte(QEMU_ICH9_LPC_ACPI_CNTL) & ~QEMU_ICH9_LPC_ACPI_CNTL_EN);
}

void qemu_ich9_lpc_acpi_sci_route(int irq) {
    lpc_write_byte(QEMU_ICH9_LPC_ACPI_CNTL, lpc_read_byte(QEMU_ICH9_LPC_ACPI_CNTL) | irq);
}

/* PCI IRQ */

void qemu_ich9_lpc_pirq_route(int pirq, int irq) {
    uint8_t offset = pirq > 3 ? QEMU_ICH9_LPC_PIRQE_ROUT + pirq - 4 : QEMU_ICH9_LPC_PIRQA_ROUT + pirq;
    lpc_write_byte(offset, (lpc_read_byte(offset) & ~QEMU_ICH9_LPC_PIRQ_MASK) | (irq & QEMU_ICH9_LPC_PIRQ_MASK));
}

void qemu_ich9_lpc_pirq_route_pic(int pirq) {
    uint8_t offset = pirq > 3 ? QEMU_ICH9_LPC_PIRQE_ROUT + pirq - 4 : QEMU_ICH9_LPC_PIRQA_ROUT + pirq;
    lpc_write_byte(offset, lpc_read_byte(offset) & ~QEMU_ICH9_LPC_PIRQ_IRQEN);
}

void qemu_ich9_lpc_pirq_route_ioapic(int pirq) {
    uint8_t offset = pirq > 3 ? QEMU_ICH9_LPC_PIRQE_ROUT + pirq - 4 : QEMU_ICH9_LPC_PIRQA_ROUT + pirq;
    lpc_write_byte(offset, lpc_read_byte(offset) | QEMU_ICH9_LPC_PIRQ_IRQEN);
}

/* Root Complex Base Address */

void qemu_ich9_lpc_rcba_set(uint32_t rcba) {
    lpc_write_dword(QEMU_ICH9_LPC_RCBA, (rcba & QEMU_ICH9_LPC_RCBA_MASK) | QEMU_ICH9_LPC_RCBA_EN);
}

void qemu_ich9_lpc_rcba_dis() {
    lpc_write_dword(QEMU_ICH9_LPC_RCBA, 0);
}
