#ifndef __MOTHERBOARD_QEMU_ICH9_LPC_H__
#define __MOTHERBOARD_QEMU_ICH9_LPC_H__

#include <stdint.h>

#define QEMU_ICH9_LPC_BUS      0
#define QEMU_ICH9_LPC_SLOT     31
#define QEMU_ICH9_LPC_FUNCTION 0

#define QEMU_ICH9_LPC_VENDOR 0x8086
#define QEMU_ICH9_LPC_DEVICE 0x2918

#define QEMU_ICH9_LPC_PMBASE      0x40
#define QEMU_ICH9_LPC_PMBASE_MASK ~0x7f

#define QEMU_ICH9_LPC_ACPI_CNTL        0x44
#define QEMU_ICH9_LPC_ACPI_CNTL_EN     (1 << 7)
#define QEMU_ICH9_LPC_ACPI_CNTL_SCI_9  0x00
#define QEMU_ICH9_LPC_ACPI_CNTL_SCI_10 0x01
#define QEMU_ICH9_LPC_ACPI_CNTL_SCI_11 0x02
#define QEMU_ICH9_LPC_ACPI_CNTL_SCI_20 0x04
#define QEMU_ICH9_LPC_ACPI_CNTL_SCI_21 0x05

#define QEMU_ICH9_LPC_PIRQA_ROUT 0x60
#define QEMU_ICH9_LPC_PIRQE_ROUT 0x68
#define QEMU_ICH9_LPC_PIRQ_IRQEN (1 << 7)

extern const uint8_t qemu_ich9_lpc_pirq_map[];

#define QEMU_ICH9_LPC_RCBA      0xf0
#define QEMU_ICH9_LPC_RCBA_MASK ~0x7fff
#define QEMU_ICH9_LPC_RCBA_EN   (1 << 0)

void qemu_ich9_lpc_pmbase(uint16_t pmbase);

void qemu_ich9_lpc_acpi_cntl_pmbase_en();
void qemu_ich9_lpc_acpi_cntl_pmbase_dis();
void qemu_ich9_lpc_acpi_sci_route(int irq);

void qemu_ich9_lpc_pirq_route(int pirq, int irq);
void qemu_ich9_lpc_pirq_route_pic(int pirq);
void qemu_ich9_lpc_pirq_route_ioapic(int pirq);

void qemu_ich9_lpc_rcba_set(uint32_t rcba);
void qemu_ich9_lpc_rcba_dis();

#endif
