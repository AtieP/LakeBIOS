#ifndef __DRIVERS_PCI_H__
#define __DRIVERS_PCI_H__

#include <stdint.h>

#define PCI_CFG_ADDRESS 0xcf8
#define PCI_CFG_DATA 0xcfc

#define PCI_CFG_VENDOR 0x00
#define PCI_CFG_COMMAND 0x04
#define PCI_CFG_HEADER 0x0e
#define PCI_CFG_BAR0 0x10
#define PCI_CFG_EXPANSION_ROM 0x26

#define PCI_CFG_HEADER_MULTIFUNCTION (1 << 7)

uint8_t pci_cfg_read_byte(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
uint16_t pci_cfg_read_word(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
uint32_t pci_cfg_read_dword(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);

void pci_cfg_write_byte(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t data);
void pci_cfg_write_word(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint16_t data);
void pci_cfg_write_dword(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t data);

void pci_enumerate();
void pci_bar_allocate(uint8_t bus, uint8_t slot, uint8_t function, int bar);

#endif
