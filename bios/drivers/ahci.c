#include <cpu/pio.h>
#include <drivers/ahci.h>
#include <drivers/pci.h>
#include <tools/alloc.h>
#include <tools/print.h>

static int s64a_supported(volatile struct ahci_abar *abar) {
    return abar->ghc.hba_capabilities & AHCI_CAP_64;
}

static int sss_supported(volatile struct ahci_abar *abar) {
    return abar->ghc.hba_capabilities & AHCI_CAP_SSS;
}

static int get_ports_silicon(volatile struct ahci_abar *abar) {
    return (abar->ghc.hba_capabilities & AHCI_CAP_PORTS_MASK) + 1;
}

static int get_slots(volatile struct ahci_abar *abar) {
    return ((abar->ghc.hba_capabilities >> 8) & AHCI_CAP_SLOTS_MASK) + 1;
}

static int port_implemented(volatile struct ahci_abar *abar, int port) {
    return abar->ghc.ports & (1 << port);
}

static int port_alloc(volatile struct ahci_abar *abar, int index) {
    volatile struct ahci_port *port = (volatile struct ahci_port *) &abar->ports[index];
    uint32_t command_list = (uint32_t) calloc(sizeof(struct ahci_command_hdr) * get_slots(abar), 1024);
    if (!command_list) {
        print("atiebios: AHCI: could not allocate command list for port %d", port);
        return -1;
    }
    uint32_t receive_fis = (uint32_t) calloc(sizeof(struct ahci_fis_hba), 256);
    if (!command_list) {
        free((void *) command_list, sizeof(struct ahci_command_hdr) * get_slots(abar));
        print("atiebios: AHCI: could not allocate receive FIS for port %d", port);
        return -1;
    }
    port->commands_list_addr_low = command_list;
    port->fis_addr_low = receive_fis;
    if (s64a_supported(abar)) {
        port->commands_list_addr_hi = 0;
        port->commands_list_addr_low = 0;
    }
    return 0;
}

static void port_free(volatile struct ahci_abar *abar, int index) {
    volatile struct ahci_port *port = (volatile struct ahci_port *) &abar->ports[index];
    free((void *) port->commands_list_addr_low, sizeof(struct ahci_command_hdr) * get_slots(abar));
    free((void *) port->fis_addr_low, sizeof(struct ahci_fis_hba));
}

static int port_init(volatile struct ahci_abar *abar, int index) {
    volatile struct ahci_port *port = (volatile struct ahci_port *) &abar->ports[index];
    // Wait for port to be done doing its things and then disable FIS receive and command execution
    while (port->command_status & (AHCI_PORT_CMD_STS_CR | AHCI_PORT_CMD_STS_FR));
    port->command_status &= (AHCI_PORT_CMD_STS_FRE | AHCI_PORT_CMD_STS_ST);
    if (port_alloc(abar, index) != 0) {
        return -1;
    }
    port->command_status |= AHCI_PORT_CMD_STS_FRE; // Otherwise, the status bits get stuck
    if (sss_supported(abar)) {
        // Staggered spinup
        port->command_status |= AHCI_PORT_CMD_STS_SUD;
        for (volatile int i = 0; i < 1000; i++) {
            inb(0x80);
        }
    }
    // Device must be brought up
    if ((port->sata_status & AHCI_PORT_SATA_STS_DET_MASK) != 3) {
        port_free(abar, index);
        return -1;
    }
    // Clear errors and wait the device for being ready
    port->sata_error |= port->sata_error;
    for (volatile int i = 0; i < 1000; i++) {
        inb(0x80);
    }
    if (port->task_file_data & (AHCI_PORT_TFD_STS_BSY | AHCI_PORT_TFD_STS_DRQ)) {
        return -1;
    }
    // Execute commands
    port->command_status |= AHCI_PORT_CMD_STS_ST;
    return 0;
}

static int controller_init(uint8_t ahci_bus, uint8_t ahci_slot, uint8_t ahci_function) {
    volatile struct ahci_abar *abar = (volatile struct ahci_abar *) (uintptr_t) pci_get_bar(ahci_bus, ahci_slot, ahci_function, 5);
    if (!abar) {
        return -1; // Very unlikely though
    }
    pci_enable_bus_mastering(ahci_bus, ahci_slot, ahci_function);
    abar->ghc.global_hba_control |= AHCI_GHC_CNT_AE;
    for (int i = 0; i < get_ports_silicon(abar); i++) {
        if (port_implemented(abar, i)) {
            if (port_init(abar, i) == 0) {
                print("atiebios: AHCI: port %d initialized successfully", i);
            } else {
                print("atiebios: AHCI: port %d could not be initialized", i);
            }
        }
    }
    return 0;
}

void ahci_init() {
    print("atiebios: AHCI: initializing controllers");
    size_t i;
    for (i = 0; i < SIZE_MAX; i++) {
        uint8_t ahci_bus;
        uint8_t ahci_slot;
        uint8_t ahci_function;
        if (pci_get_device(AHCI_CLASS, AHCI_SUBCLASS, AHCI_INTERFACE, &ahci_bus, &ahci_slot, &ahci_function, i) == 0) {
            print("atiebios: AHCI: controller found at bus %d slot %d function %d", ahci_bus, ahci_slot, ahci_function);
            controller_init(ahci_bus, ahci_slot, ahci_function);
        }
        break;
    }
    print("atiebios: AHCI: finished initializing controllers");
}
