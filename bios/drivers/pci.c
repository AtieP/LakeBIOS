#include <cpu/pio.h>
#include <drivers/pci.h>
#include <tools/print.h>

static uintptr_t bar_mmio_base = 0xc0000000;
static uint16_t bar_io_base = 0xc000;

static void send_address(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    outd(PCI_CFG_ADDRESS, 0x80000000 | (bus << 16) | (slot << 11) | (function << 8) | (offset & 0xfc));
}

uint8_t pci_cfg_read_byte(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    send_address(bus, slot, function, offset);
    return inb(PCI_CFG_DATA + (offset & 3));
}

uint16_t pci_cfg_read_word(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    send_address(bus, slot, function, offset);
    return inw(PCI_CFG_DATA + (offset & 2));
}

uint32_t pci_cfg_read_dword(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    send_address(bus, slot, function, offset);
    return ind(PCI_CFG_DATA);
}

void pci_cfg_write_byte(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t data) {
    send_address(bus, slot, function, offset);
    outb(PCI_CFG_DATA + (offset & 3), data);
}

void pci_cfg_write_word(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint16_t data) {
    send_address(bus, slot, function, offset);
    outw(PCI_CFG_DATA + (offset & 2), data);
}

void pci_cfg_write_dword(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t data) {
    send_address(bus, slot, function, offset);
    outd(PCI_CFG_DATA, data);
}

void pci_enable_memory(uint8_t bus, uint8_t slot, uint8_t function) {
    pci_cfg_write_word(bus, slot, function, PCI_CFG_COMMAND, pci_cfg_read_word(bus, slot, function, PCI_CFG_COMMAND) | PCI_CFG_COMMAND_MEM_ENABLE);
}

void pci_enable_io(uint8_t bus, uint8_t slot, uint8_t function) {
    pci_cfg_write_word(bus, slot, function, PCI_CFG_COMMAND, pci_cfg_read_word(bus, slot, function, PCI_CFG_COMMAND) | PCI_CFG_COMMAND_IO_ENABLE);
}

void pci_disable_memory(uint8_t bus, uint8_t slot, uint8_t function) {
    pci_cfg_write_word(bus, slot, function, PCI_CFG_COMMAND, pci_cfg_read_word(bus, slot, function, PCI_CFG_COMMAND) & ~PCI_CFG_COMMAND_MEM_ENABLE);

}

void pci_disable_io(uint8_t bus, uint8_t slot, uint8_t function) {
    pci_cfg_write_word(bus, slot, function, PCI_CFG_COMMAND, pci_cfg_read_word(bus, slot, function, PCI_CFG_COMMAND) & ~PCI_CFG_COMMAND_IO_ENABLE);

}

static void pci_setup_function(uint8_t bus, uint8_t slot, uint8_t function) {
    uint16_t vendor_id = pci_cfg_read_word(bus, slot, function, PCI_CFG_VENDOR);
    if (vendor_id == 0xffff || vendor_id == 0x0000) {
        return;
    }
    // TODO: all the setup it requires...
    // Allocate BARs
    for (int bar = 0; bar <= 6; bar++) {
        if (pci_bar_allocate(bus, slot, function, bar) == 2) {
            bar++;
        }
    }
}

static void pci_enumerate_slot(uint8_t bus, uint8_t slot) {
    uint16_t vendor_id = pci_cfg_read_word(bus, slot, 0, PCI_CFG_VENDOR);
    if (vendor_id == 0xffff || vendor_id == 0x0000) {
        return;
    }
    uint8_t functions;
    if (pci_cfg_read_byte(bus, slot, 0, PCI_CFG_HEADER) & PCI_CFG_HEADER_MULTIFUNCTION) {
        functions = 8;
    } else {
        functions = 1;
    }
    for (uint8_t function = 0; function < functions; function++) {
        pci_setup_function(bus, slot, function);
    }
}

static void pci_enumerate_bus(uint8_t bus) {
    for (uint8_t slot = 0; slot < 32; slot++) {
        pci_enumerate_slot(bus, slot);
    }
}

void pci_enumerate() {
    // TODO: if there are more root buses, enumerate them too
    pci_enumerate_bus(0);
}

// Returns: bar kind
int pci_bar_allocate(uint8_t bus, uint8_t slot, uint8_t function, int bar) {
    int bar_offset;
    if (bar == 6) {
        // Do not handle expansion ROMs
        return -1;
    } else {
        bar_offset = PCI_CFG_BAR0 + (bar * 4);
    }
    uint32_t bar_original_value = pci_cfg_read_dword(bus, slot, function, bar_offset);
    int kind; // 0: 32-bit MMIO BAR, 1: IO BAR, 2: 64-bit MMIO BAR
    if (bar_original_value & 1) {
        // IO BAR
        kind = 1;
    } else {
        if (((bar_original_value >> 1) & 0b11) == 0) {
            // 32-bit MMIO BAR
            kind = 0;
        } else if (((bar_original_value >> 1) & 0b11) == 2) {
            kind = 2;
        } else {
            return -1; // Invalid MMIO BAR kind, 16-bit MMIO bars are forbidden
        }
    }
    uint32_t bar_size;
    pci_cfg_write_dword(bus, slot, function, bar_offset, ~0);
    if (kind == 1) {
        // IO BAR
        bar_size = pci_cfg_read_dword(bus, slot, function, bar_offset);
        bar_size &= ~0b11;
        bar_size = ~bar_size + 1;
    } else if (kind == 0) {
        // 32-bit MMIO BAR
        bar_size = pci_cfg_read_dword(bus, slot, function, bar_offset);
        bar_size &= ~0b1111;
        bar_size = ~bar_size + 1;
    } else if (kind == 2) {
        bar_size = pci_cfg_read_dword(bus, slot, function, bar_offset);
        bar_size &= ~0b1111;
        bar_size = ~bar_size + 1;
    }
    pci_cfg_write_dword(bus, slot, function, bar_offset, bar_original_value);
    // If BAR doesn't exist, size is 0
    if (bar_size == 0) {
        return -1;
    }
    if (kind == 1) {
        pci_cfg_write_dword(bus, slot, function, bar_offset, bar_io_base);
        bar_io_base += bar_size;
        pci_enable_io(bus, slot, function);
    } else if (kind == 0) {
        pci_cfg_write_dword(bus, slot, function, bar_offset, bar_mmio_base);
        bar_mmio_base += (bar_size + 4095) & ~4095;
        pci_enable_memory(bus, slot, function);
    } else if (kind == 2) {
        pci_cfg_write_dword(bus, slot, function, bar_offset, bar_mmio_base);
        pci_cfg_write_dword(bus, slot, function, bar_offset + 4, 0);
        bar_mmio_base += (bar_size + 4095) & ~4095;
        pci_enable_memory(bus, slot, function);
    }
    return kind;
}
