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

static int get_free_slot(volatile struct ahci_abar *abar, int index) {
    uint32_t slots = abar->ports[index].sata_active | abar->ports[index].command_issue;
    for (int i = 0; i < get_slots(abar); i++) {
        if (!(slots & (1 << i))) {
            return i;
        }
    }
    return 0;
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
    port->command_status &= ~(AHCI_PORT_CMD_STS_FRE | AHCI_PORT_CMD_STS_ST);
    port->interrupt_enable = 0;
    port->interrupt_status = 0xffffffff;
    if (port_alloc(abar, index) != 0) {
        return -1;
    }
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
    port->command_status |= AHCI_PORT_CMD_STS_FRE; // Otherwise, the status bits get stuck
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
            if (controller_init(ahci_bus, ahci_slot, ahci_function) == -1) {
                print("atiebios: AHCI: controller mentioned before has not been initialized successfully");
            } else {
                print("atiebios: AHCI: controller mentioned before has been initialized successfully");
            }
        } else {
            break;
        }
    }
    print("atiebios: AHCI: finished initializing controllers");
}

int ahci_send_command(volatile struct ahci_abar *abar, int index, uint8_t command, void *buf, long long lba, int len, int write, int atapi) {
    if (atapi) {
        return -1; // todo: support it later
    }
    int slot = get_free_slot(abar, index);
    if (slot == -1) {
        return -1;
    }
    volatile struct ahci_port *port = &abar->ports[index];
    struct ahci_command_hdr *cmd_hdr = (struct ahci_command_hdr *) port->commands_list_addr_low;
    cmd_hdr += slot;
    struct ahci_command_tbl *cmd_tbl = (struct ahci_command_tbl *) calloc(sizeof(struct ahci_command_tbl) + sizeof(struct ahci_prdt), 128);
    if (!cmd_tbl) {
        return -1;
    }
    cmd_tbl->command_fis.fis_kind = AHCI_FIS_H2D; // H2D
    cmd_tbl->command_fis.flags = 1 << 7;
    cmd_tbl->command_fis.command = command;
    if (lba != -1) {
        cmd_tbl->command_fis.lba0 = (uint8_t) lba;
        cmd_tbl->command_fis.lba1 = (uint8_t) (lba >> 8);
        cmd_tbl->command_fis.lba2 = (uint8_t) (lba >> 16);
        cmd_tbl->command_fis.device = (1 << 6) | 0xa0;
        cmd_tbl->command_fis.lba3 = (uint8_t) (lba >> 24);
        cmd_tbl->command_fis.lba4 = (uint8_t) (lba >> 32);
        cmd_tbl->command_fis.lba5 = (uint8_t) (lba >> 48);
        cmd_tbl->command_fis.count_low = (uint8_t) (len / 512);
        cmd_tbl->command_fis.count_hi = (uint8_t) ((len / 512) >> 8);
    }
    cmd_tbl->prdt[0].data_addr_low = (uint32_t) buf;
    cmd_tbl->prdt[0].data_addr_hi = 0;
    cmd_tbl->prdt[0].description = len - 1;
    uint32_t cmd_hdr_flags =
          (1 << AHCI_CMD_HDR_FLAGS_PRDTL_SHIFT)
        | (1 << 10)
        | (write ? AHCI_CMD_HDR_FLAGS_W : 0)
        | (atapi ? AHCI_CMD_HDR_FLAGS_ATAPI : 0)
        | (sizeof(struct ahci_fis_h2d) / 4)
    ;
    cmd_hdr->command_table_low = (uint32_t) cmd_tbl;
    cmd_hdr->command_table_hi = 0;
    cmd_hdr->flags = cmd_hdr_flags;
    // Wait for other commands to complete and issue command
    port->interrupt_status = 0xffffffff;
    while (port->task_file_data & (AHCI_PORT_TFD_STS_BSY | AHCI_PORT_TFD_STS_DRQ));
    port->command_issue |= (1 << slot);
    while (port->command_issue & (1 << slot));
    free((void *) cmd_tbl, sizeof(struct ahci_command_tbl) + sizeof(struct ahci_prdt));
    return 0;
}
