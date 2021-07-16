#ifndef __DRIVERS_PCI_H__
#define __DRIVERS_PCI_H__

#include <stddef.h>
#include <stdint.h>

#define PCI_CFG_ADDRESS 0xcf8
#define PCI_CFG_DATA 0xcfc

#define PCI_CFG_VENDOR 0x00
#define PCI_CFG_DEVICE 0x02
#define PCI_CFG_COMMAND 0x04
#define PCI_CFG_INTERFACE 0x09
#define PCI_CFG_SUBCLASS 0x0a
#define PCI_CFG_CLASS 0x0b
#define PCI_CFG_HEADER 0x0e
#define PCI_CFG_BAR0 0x10
#define PCI_CFG_EXPANSION_ROM 0x26
#define PCI_CFG_INTERRUPT_LINE 0x3c
#define PCI_CFG_INTERRUPT_PIN 0x3d

#define PCI_CFG_COMMAND_IO_ENABLE (1 << 0)
#define PCI_CFG_COMMAND_MEM_ENABLE (1 << 1)
#define PCI_CFG_COMMAND_DMA_ENABLE (1 << 2)
#define PCI_CFG_COMMAND_INT_DISABLE (1 << 10)

#define PCI_CFG_HEADER_MULTIFUNCTION (1 << 7)

uint8_t pci_cfg_read_byte(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
uint16_t pci_cfg_read_word(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
uint32_t pci_cfg_read_dword(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);

void pci_cfg_write_byte(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t data);
void pci_cfg_write_word(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint16_t data);
void pci_cfg_write_dword(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t data);

void pci_enable_memory(uint8_t bus, uint8_t slot, uint8_t function);
void pci_disable_memory(uint8_t bus, uint8_t slot, uint8_t function);
void pci_enable_io(uint8_t bus, uint8_t slot, uint8_t function);
void pci_disable_io(uint8_t bus, uint8_t slot, uint8_t function);
void pci_enable_bus_mastering(uint8_t bus, uint8_t slot, uint8_t function);
void pci_disable_bus_mastering(uint8_t bus, uint8_t slot, uint8_t function);
void pci_enable_interrupts(uint8_t bus, uint8_t slot, uint8_t function);
void pci_disable_interrupts(uint8_t bus, uint8_t slot, uint8_t function);

void pci_enumerate(uintptr_t mmio_base, uintptr_t io_base, uint8_t pirqa_line, uint8_t pirqb_line, uint8_t pirqc_line, uint8_t pirqd_line);
int pci_bar_allocate(uint8_t bus, uint8_t slot, uint8_t function, int bar, uintptr_t mmio_base, uintptr_t io_base);
int pci_get_device(uint8_t class, uint8_t subclass, uint8_t interface, uint8_t *bus_ptr, uint8_t *slot_ptr, uint8_t *function_ptr, size_t index);
uint64_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t function, int bar);

#endif
