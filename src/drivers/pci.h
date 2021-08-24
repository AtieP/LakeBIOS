#ifndef __DRIVERS_PCI_H__
#define __DRIVERS_PCI_H__

#include <stddef.h>
#include <stdint.h>

#define PCI_CFG_ADDRESS 0xcf8
#define PCI_CFG_DATA    0xcfc

#define PCI_CFG_VENDOR         0x00
#define PCI_CFG_DEVICE         0x02
#define PCI_CFG_COMMAND        0x04
#define PCI_CFG_INTERFACE      0x09
#define PCI_CFG_SUBCLASS       0x0a
#define PCI_CFG_CLASS          0x0b
#define PCI_CFG_HEADER         0x0e
#define PCI_CFG_BAR0           0x10
#define PCI_CFG_EXPANSION_ROM  0x30
#define PCI_CFG_INTERRUPT_LINE 0x3c
#define PCI_CFG_INTERRUPT_PIN  0x3d

/* The following batched defines are only for PCI bridges */
#define PCI_CFG_PRIMARY_BUS       0x18
#define PCI_CFG_SECONDARY_BUS     0x19
#define PCI_CFG_SUBORDINATE_BUS   0x1a
#define PCI_CFG_IO_BASE           0x1c
#define PCI_CFG_IO_LIMIT          0x1d
#define PCI_CFG_MEMORY_BASE       0x20
#define PCI_CFG_MEMORY_LIMIT      0x22
#define PCI_CFG_PREFETCH_BASE     0x24
#define PCI_CFG_PREFETCH_LIMIT    0x26
#define PCI_CFG_PREFETCH_BASE_HI  0x28
#define PCI_CFG_PREFETCH_LIMIT_HI 0x2c
#define PCI_CFG_IO_BASE_HI        0x30
#define PCI_CFG_IO_LIMIT_HI       0x32

#define PCI_CFG_COMMAND_IO_ENABLE   (1 << 0)
#define PCI_CFG_COMMAND_MEM_ENABLE  (1 << 1)
#define PCI_CFG_COMMAND_BUS_MASTER  (1 << 2)
#define PCI_CFG_COMMAND_INT_DISABLE (1 << 10)

#define PCI_CFG_HEADER_MULTIFUNCTION (1 << 7)

#define PCI_BAR_MEM_32  0x00
#define PCI_BAR_IO      0x01
#define PCI_BAR_MEM_64  0x02
#define PCI_BAR_PREF_32 0x04
#define PCI_BAR_PREF_64 0x06

uint8_t pci_cfg_read_byte(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
uint16_t pci_cfg_read_word(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
uint32_t pci_cfg_read_dword(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);

void pci_cfg_write_byte(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t data);
void pci_cfg_write_word(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint16_t data);
void pci_cfg_write_dword(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t data);

void pci_control_set(uint8_t bus, uint8_t slot, uint8_t function, uint16_t bits);
void pci_control_clear(uint8_t bus, uint8_t slot, uint8_t function, uint16_t bits);

int pci_setup(uintptr_t mem_base_, uintptr_t mem_limit_, uint16_t io_base_, uint16_t io_limit_, uintptr_t pref_base_, uintptr_t pref_limit_, uint8_t (*get_interrupt_line_)(int pirq, uint8_t bus, uint8_t slot, uint8_t function));
int pci_get_device(uint8_t class, uint8_t subclass, uint8_t interface, uint8_t *bus_ptr, uint8_t *slot_ptr, uint8_t *function_ptr, size_t index);
uint64_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t function, int bar);

#endif
