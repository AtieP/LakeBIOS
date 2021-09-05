#include <cpu/pio.h>
#include <drivers/bus/pci.h>
#include <tools/print.h>

/* Global variables */
static uintptr_t mem_base;
static uintptr_t mem_limit;
static uint16_t io_base;
static uint16_t io_limit;
static uintptr_t pref_base;
static uintptr_t pref_limit;

static uint8_t (*get_interrupt_line)(int pin, uint8_t bus, uint8_t slot, uint8_t function);
static int buses = 1;

/* Utilities */

static void send_address(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    outd(PCI_CFG_ADDRESS, 0x80000000 | (bus << 16) | (slot << 11) | (function << 8) | (offset & 0xfc));
}

static int pci_exists() {
    send_address(0, 0, 0, 0);
    if (ind(PCI_CFG_ADDRESS) != 0x80000000) {
        return 0;
    }
    return 1;
}

static int get_bar_type(uint32_t bar) {
    if (bar & 1) {
        return PCI_BAR_IO;
    }
    return (bar & 0x0f) >> 1;
}

static int allocate_bus() {
    return buses++;
}

static int allocate_bar(uint8_t bus, uint8_t slot, uint8_t function, int bar) {
    int offset = PCI_CFG_BAR0 + (bar * 4);
    uint64_t bar_orig_value = (uint64_t) pci_cfg_read_dword(bus, slot, function, offset);
    uint64_t bar_size = 0;
    int type = get_bar_type((uint32_t) bar_orig_value);
    if (type == PCI_BAR_IO) {
        pci_cfg_write_dword(bus, slot, function, offset, 0xffffffff);
        bar_size = (uint64_t) pci_cfg_read_dword(bus, slot, function, offset);
        bar_size &= ~0x03;
        bar_size = ~((uint32_t) bar_size) + 1;
        pci_cfg_write_dword(bus, slot, function, offset, (uint32_t) bar_orig_value);
    } else if (type == PCI_BAR_MEM_32 || type == PCI_BAR_PREF_32) {
        pci_cfg_write_dword(bus, slot, function, offset, 0xffffffff);
        bar_size = pci_cfg_read_dword(bus, slot, function, offset);
        bar_size &= ~0x0f;
        bar_size = ~((uint32_t) bar_size) + 1;
        pci_cfg_write_dword(bus, slot, function, offset, (uint32_t) bar_orig_value);
    } else if (type == PCI_BAR_MEM_64 || type == PCI_BAR_PREF_64) {
        bar_orig_value |= ((uint64_t) pci_cfg_read_dword(bus, slot, function, offset + 4) << 32);
        // Lower half of size
        pci_cfg_write_dword(bus, slot, function, offset, 0xffffffff);
        bar_size = pci_cfg_read_dword(bus, slot, function, offset);
        bar_size &= ~0x0f;
        bar_size = ~((uint32_t) bar_size) + 1;
        pci_cfg_write_dword(bus, slot, function, offset, (uint32_t) bar_orig_value);
        // Higher half of size
        pci_cfg_write_dword(bus, slot, function, offset + 4, 0xffffffff);
        bar_size |= (uint64_t) ~pci_cfg_read_dword(bus, slot, function, offset + 4) << 32;
        pci_cfg_write_dword(bus, slot, function, offset + 4, (uint32_t) (bar_orig_value >> 32));
    }
    // Does the BAR exist?
    if (!bar_size) {
        return -1;
    }
    if (type == PCI_BAR_IO) {
        uint64_t temp_io_base = io_base;
        temp_io_base = (temp_io_base + (bar_size - 1)) & ~(bar_size - 1);
        if (temp_io_base + bar_size > (uint64_t) io_limit) {
            print("PCI: Cannot allocate IO BAR #%d of Bus %d Slot %d Function %d because there isn't enough space", bar, bus, slot, function);
            return type;
        }
        pci_cfg_write_dword(bus, slot, function, offset, temp_io_base);
        io_base = temp_io_base + bar_size;
    }
    if (type == PCI_BAR_MEM_32 || type == PCI_BAR_MEM_64) {
        uint64_t temp_mem_base = (uint64_t) mem_base;
        temp_mem_base = (temp_mem_base + (bar_size - 1)) & ~(bar_size - 1);
        if (temp_mem_base + bar_size > (uint64_t) mem_limit) {
            print("PCI: Cannot allocate Memory BAR #%d of Bus %d Slot %d Function %d because there isn't enough space", bar, bus, slot, function);
            return type;
        }
        pci_cfg_write_dword(bus, slot, function, offset, temp_mem_base);
        if (type == PCI_BAR_MEM_64) {
            pci_cfg_write_dword(bus, slot, function, offset + 4, 0);
        }
        mem_base = temp_mem_base + bar_size;
    }
    if (type == PCI_BAR_PREF_32 || type == PCI_BAR_PREF_64){
        uint64_t temp_pref_base = (uint64_t) pref_base;
        temp_pref_base = (temp_pref_base + (bar_size - 1)) & ~(bar_size - 1);
        if (temp_pref_base + bar_size > (uint64_t) pref_limit) {
            print("PCI: Cannot allocate Prefetchable BAR #%d of Bus %d Slot %d Function %d because there isn't enough space", bar, bus, slot, function);
            return type;
        }
        pci_cfg_write_dword(bus, slot, function, offset, temp_pref_base);
        if (type == PCI_BAR_PREF_64) {
            pci_cfg_write_dword(bus, slot, function, offset + 4, 0);
        }
        pref_base = temp_pref_base + bar_size;
    }
    return type;
}

static void setup_device(uint8_t bus, uint8_t slot, uint8_t function) {
    print("PCI: Device found on Bus %d Slot %d Function %d", bus, slot, function);
    for (int i = 0; i < 6; i++) {
        int type = allocate_bar(bus, slot, function, i);
        if (type == PCI_BAR_MEM_64 || type == PCI_BAR_PREF_64) {
            i++;
        }
    }
}

static int setup_bus(uint8_t bus);

static void setup_pci_bridge(uint8_t bus, uint8_t slot, uint8_t function, int *found_buses) {
    print("PCI: PCI bridge found on Bus %d Slot %d Function %d", bus, slot, function);
    // Allocate those 2 BARs
    for (int i = 0; i < 2; i++) {
        int type = allocate_bar(bus, slot, function, i);
        if (type == PCI_BAR_MEM_64 || type == PCI_BAR_PREF_64) {
            break;
        }
    }
    // Figure out what kind of memory the bridge accepts
    int io_implemented = 1;
    int mem_implemented = 1;
    int pref_implemented = 1;
    pci_cfg_write_byte(bus, slot, function, PCI_CFG_IO_BASE, 0xff);
    if (pci_cfg_read_byte(bus, slot, function, PCI_CFG_IO_BASE) == 0x00) {
        io_implemented = 0;
    }
    pci_cfg_write_word(bus, slot, function, PCI_CFG_MEMORY_BASE, 0xffff);
    if (pci_cfg_read_word(bus, slot, function, PCI_CFG_MEMORY_BASE) == 0x0000) {
        mem_implemented = 0;
    }
    pci_cfg_write_word(bus, slot, function, PCI_CFG_PREFETCH_BASE, 0xffff);
    if (pci_cfg_read_word(bus, slot, function, PCI_CFG_PREFETCH_BASE) == 0x0000) {
        pref_implemented = 0;
    }
    // Align memory, IO and prefetchable bases
    if (io_implemented) {
        io_base = (io_base + 0x0fff) & ~0x0fff;
    }
    if (mem_implemented) {
        mem_base = (mem_base + 0xffffe) & ~0xffffe;
    }
    if (pref_implemented) {
        pref_base = (pref_base + 0xffffe) & ~0xffffe;
    }
    uintptr_t orig_mem_base = mem_base;
    uint16_t orig_io_base = io_base;
    uintptr_t orig_pref_base = pref_base;
    // Assign bus numbers
    int new_bus = allocate_bus();
    pci_cfg_write_byte(bus, slot, function, PCI_CFG_PRIMARY_BUS, bus);
    pci_cfg_write_byte(bus, slot, function, PCI_CFG_SECONDARY_BUS, new_bus);
    pci_cfg_write_byte(bus, slot, function, PCI_CFG_SUBORDINATE_BUS, 0xff);
    pci_cfg_write_byte(bus, slot, function, PCI_CFG_SUBORDINATE_BUS, new_bus + setup_bus(new_bus));
    // Tell the bridge what ranges it can use
    if (io_implemented) {
        pci_cfg_write_byte(bus, slot, function, PCI_CFG_IO_BASE, (orig_io_base >> 11) << 3);
        pci_cfg_write_byte(bus, slot, function, PCI_CFG_IO_LIMIT, (io_base >> 11) << 3);
        pci_cfg_write_word(bus, slot, function, PCI_CFG_IO_BASE_HI, 0);
        pci_cfg_write_word(bus, slot, function, PCI_CFG_IO_LIMIT_HI, 0);
        io_base++;
        io_base = (io_base + 0x0fff) & ~0x0fff;
    }
    if (mem_implemented) {
        pci_cfg_write_word(bus, slot, function, PCI_CFG_MEMORY_BASE, (orig_mem_base >> 19) << 3);
        pci_cfg_write_word(bus, slot, function, PCI_CFG_MEMORY_LIMIT, (mem_base >> 19) << 3);
        mem_base++;
        mem_base = (mem_base + 0xffffe) & ~0xffffe;
    }
    if (pref_implemented) {
        pci_cfg_write_word(bus, slot, function, PCI_CFG_PREFETCH_BASE, (orig_pref_base >> 19) << 3);
        pci_cfg_write_word(bus, slot, function, PCI_CFG_PREFETCH_LIMIT, (pref_base >> 19) << 3);
        pci_cfg_write_dword(bus, slot, function, PCI_CFG_PREFETCH_BASE_HI, 0);
        pci_cfg_write_dword(bus, slot, function, PCI_CFG_PREFETCH_LIMIT_HI, 0);
        pref_base++;
        pref_base = (pref_base + 0xffffe) & ~0xffffe;
    }
    *found_buses = *found_buses + 1;
}

static void setup_cardbus_bridge(uint8_t bus, uint8_t slot, uint8_t function) {
    print("PCI: Cardbus bridge found on Bus %d Slot %d Function %d, ignoring", bus, slot, function);
}

static void setup_function(uint8_t bus, uint8_t slot, uint8_t function, int *found_buses) {
    if (pci_cfg_read_word(bus, slot, function, PCI_CFG_VENDOR) == 0xffff) {
        return;
    }
    uint8_t header = pci_cfg_read_byte(bus, slot, function, PCI_CFG_HEADER) & ~PCI_CFG_HEADER_MULTIFUNCTION;
    pci_control_set(bus, slot, function, PCI_CFG_COMMAND_MEM_ENABLE | PCI_CFG_COMMAND_IO_ENABLE);
    pci_cfg_write_byte(bus, slot, function, PCI_CFG_INTERRUPT_LINE, get_interrupt_line(
        pci_cfg_read_byte(bus, slot, function, PCI_CFG_INTERRUPT_PIN), bus, slot, function
    ));
    if (header == 0x00) {
        setup_device(bus, slot, function);
    } else if (header == 0x01) {
        setup_pci_bridge(bus, slot, function, found_buses);
    } else if (header == 0x02) {
        setup_cardbus_bridge(bus, slot, function);
    } else {
        print("PCI: Invalid header type for Bus %d Slot %d Function %d", bus, slot, function);
    }
}

static void setup_slot(uint8_t bus, uint8_t slot, int *found_buses) {
    if (pci_cfg_read_word(bus, slot, 0, PCI_CFG_VENDOR) == 0xffff) {
        return;
    }
    int functions = pci_cfg_read_byte(bus, slot, 0, PCI_CFG_HEADER) & PCI_CFG_HEADER_MULTIFUNCTION ? 8 : 1;
    for (int i = 0; i < functions; i++) {
        setup_function(bus, slot, i, found_buses);
    }
}

static int setup_bus(uint8_t bus) {
    int found_buses = 0;
    for (int i = 0; i < 32; i++) {
        setup_slot(bus, i, &found_buses);
    }
    return found_buses;
}

/* Globally visible functions */

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

void pci_control_set(uint8_t bus, uint8_t slot, uint8_t function, uint16_t bits) {
    pci_cfg_write_word(bus, slot, function, PCI_CFG_COMMAND, pci_cfg_read_word(bus, slot, function, PCI_CFG_COMMAND) | bits);
}

void pci_control_clear(uint8_t bus, uint8_t slot, uint8_t function, uint16_t bits) {
    pci_cfg_write_word(bus, slot, function, PCI_CFG_COMMAND, pci_cfg_read_word(bus, slot, function, PCI_CFG_COMMAND) & ~bits);
}

int pci_setup(uintptr_t mem_base_, uintptr_t mem_limit_, uint16_t io_base_, uint16_t io_limit_, uintptr_t pref_base_, uintptr_t pref_limit_, uint8_t (*get_interrupt_line_)(int pin, uint8_t bus, uint8_t slot, uint8_t function)) {
    if (!pci_exists()) {
        print("PCI: Not available");
        return -1;
    }
    mem_base = mem_base_;
    io_base = io_base_;
    pref_base = pref_base_;
    mem_limit = mem_limit_;
    io_limit = io_limit_;
    pref_limit = pref_limit_;
    get_interrupt_line = get_interrupt_line_;
    setup_bus(0);
    return 0;
}

int pci_get_device(uint8_t class, uint8_t subclass, uint8_t interface, uint8_t *bus_ptr, uint8_t *slot_ptr, uint8_t *function_ptr, size_t index) {
    size_t matches = 0;
    for (int bus = 0; bus < buses; bus++) {
        for (int slot = 0; slot < 32; slot++) {
            if (pci_cfg_read_word(bus, slot, 0, PCI_CFG_VENDOR) == 0xffff) {
                continue;
            }
            int functions = pci_cfg_read_byte(bus, slot, 0, PCI_CFG_HEADER) & PCI_CFG_HEADER_MULTIFUNCTION ? 8 : 1;
            for (int function = 0; function < functions; function++) {
                if (pci_cfg_read_word(bus, slot, function, PCI_CFG_VENDOR) == 0xffff) {
                    continue;
                }
                uint8_t class_ = pci_cfg_read_byte(bus, slot, function, PCI_CFG_CLASS);
                uint8_t subclass_ = pci_cfg_read_byte(bus, slot, function, PCI_CFG_SUBCLASS);
                uint8_t interface_ = pci_cfg_read_byte(bus, slot, function, PCI_CFG_INTERFACE);
                if (class == class_ && subclass == subclass_ && interface == interface_) {
                    if (index == matches) {
                        *bus_ptr = bus;
                        *slot_ptr = slot;
                        *function_ptr = function;
                        return 0;
                    }
                    matches++;
                }
            }
        }
    }
    return -1;
}

uint64_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t function, int bar) {
    uint32_t bar_val = pci_cfg_read_dword(bus, slot, function, PCI_CFG_BAR0 + (bar * 4));
    if (bar_val & 1) {
        // I/O bar
        return (uint64_t) ((uint16_t) bar_val & ~0x03);
    }
    if (((bar_val >> 1) & 0x03) == 0) {
        // 32-bit MMIO bar
        return (uint64_t) (bar_val & ~0x0f);
    }
    if (((bar_val >> 1) & 0x03) == 2) {
        // 64-bit MMIO bar
        return ((uint64_t) pci_cfg_read_dword(bus, slot, function, PCI_CFG_BAR0 + ((bar + 1) * 4)) << 32) | (bar_val & ~0x0f);
    }
    return 0;
}
