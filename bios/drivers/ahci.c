#include <cpu/pio.h>
#include <drivers/pci.h>
#include <drivers/ahci.h>
#include <tools/alloc.h>
#include <tools/print.h>

static volatile struct ahci_abar *abar;
static int ahci_available_ports;
static int ahci_available_cmd_slots;
static int ahci_64_bit;
static uint8_t ahci_bus;
static uint8_t ahci_slot;
static uint8_t ahci_function;

static int get_free_slot(int port) {
    uint32_t slots = abar->ports[port].sata_active | abar->ports[port].command_issue;
    for (int i = 0; i < ahci_available_cmd_slots; i++) {
        if (!(slots & (1 << i))) {
            return i;
        }
    }
    return -1;
}

int ahci_detect() {
    return pci_get_device(AHCI_CLASS, AHCI_SUBCLASS, AHCI_INTERFACE, &ahci_bus, &ahci_slot, &ahci_function);
}

// Gathers important information about AHCI
int ahci_early_setup() {
    if (pci_get_device(AHCI_CLASS, AHCI_SUBCLASS, AHCI_INTERFACE, &ahci_bus, &ahci_slot, &ahci_function) != 0) {
        return -1; // AHCI not supported
    }
    abar = (volatile struct ahci_abar *) (uintptr_t)pci_get_bar(ahci_bus, ahci_slot, ahci_function, 5);
    if (abar == 0) {
        return -1; // No ABAR (very unlikely)
    }
    ahci_available_ports = (abar->ghc.hba_capabilities & AHCI_CAP_PORTS_MASK) + 1;
    ahci_available_cmd_slots = ((abar->ghc.hba_capabilities >> 8) & AHCI_CAP_SLOTS_MASK) + 1;
    print("atiebios: AHCI: %d ports and %d command slots available", ahci_available_ports, ahci_available_cmd_slots);
    ahci_64_bit = (abar->ghc.hba_capabilities & AHCI_CAP_64);
    if (ahci_64_bit) {
        print("atiebios: AHCI: 64-bit addressing supported");
    }
    pci_enable_bus_mastering(ahci_bus, ahci_slot, ahci_function);
    return 0;
}

// BIOS initialization for the HBA
int ahci_setup() {
    // Mechanical presence switch and staggered spinup: false
    for (int i = 0; i < ahci_available_ports; i++) {
        // Hot plug capable: false
        // Mechanical presence: false
        // Cold presence detect logic: false
        // DEVSLP logic: false
    }
    // Software is AHCI aware
    abar->ghc.global_hba_control |= AHCI_GHC_CNT_AE;
    for (int i = 0; i < ahci_available_ports; i++) {
        // Wait for command to stop running and receive FIS transfers
        while (abar->ports[i].command_status & (AHCI_PORT_CMD_STS_CR | AHCI_PORT_CMD_STS_FR));
        abar->ports[i].command_status &= ~(AHCI_PORT_CMD_STS_FRE | AHCI_PORT_CMD_STS_ST);
        abar->ports[i].sata_control &= ~(0b1111); // DET
        // Allocate command list and receive FIS
        uint32_t command_list = (uint32_t) calloc(sizeof(struct ahci_command_hdr) * ahci_available_cmd_slots, 1024);
        if (!command_list) {
            print("atiebios: AHCI: could not allocate command list for port %d", i);
        }
        uint32_t receive_fis = (uint32_t) calloc(sizeof(struct ahci_fis_hba), 256);
        if (!receive_fis) {
            free((void *) command_list, sizeof(struct ahci_command_hdr) * ahci_available_cmd_slots);
            print("atiebios: AHCI: could not allocate receive FIS for port %d", i);
        }
        abar->ports[i].commands_list_addr_low = command_list;
        abar->ports[i].fis_addr_low = receive_fis;
        if (ahci_64_bit) {
            abar->ports[i].commands_list_addr_hi = 0;
            abar->ports[i].fis_addr_low = 0;
        }
        abar->ports[i].command_status |= AHCI_PORT_CMD_STS_FRE;
        // No need for staggered spinup, not supported
        // Wait some time to let the hardware decide if the port is actually connected
        for (int i = 0; i < 1000; i++) {
            inb(0x80);
        }
        // Determine connection status of the port
        uint8_t det = abar->ports[i].sata_status & 0b1111;
        if (det == 0) {
            print("atiebios: AHCI: no device detected and no communication established at port %d", i);
            continue;
        }
        if (det == 1) {
            print("atiebios: AHCI: device detected but no communication established at port %d", i);
        }
        if (det == 3) {
            print("atiebios: AHCI: device detected and communication established at port %d", i);
        }
        if (det == 4) {
            print("atiebios: AHCI: phy in offline mode at port %d", i);
            continue;
        }
        // Clear errors and poll
        abar->ports[i].sata_error = 0xffffffff;
        while (abar->ports[i].task_file_data & (AHCI_PORT_TFD_STS_BSY | AHCI_PORT_TFD_STS_DRQ));
        abar->ports[i].command_status |= AHCI_PORT_CMD_STS_ST;
    }
    return 0;
}

int ahci_send_command(int port, uint8_t command, void *buf, uint64_t lba, int count, int write) {
    int slot = get_free_slot(port);
    if (slot == -1) {
        return -1;
    }
    abar->ports[port].interrupt_status = -1;
    struct ahci_command_hdr *command_hdr = (struct ahci_command_hdr *) abar->ports[port].commands_list_addr_low;
    command_hdr += slot;
    struct ahci_command_table *command_table = calloc(sizeof(struct ahci_command_table), 128);
    if (!command_table) {
        return -1;
    }
    command_table->command_fis.fis_kind = 0x27;
    command_table->command_fis.command = command;
    command_table->command_fis.flags = 1 << 7;
    command_table->command_fis.device = 1 << 6; // LBA
    command_table->command_fis.lba0 = (uint8_t) lba;
    command_table->command_fis.lba1 = (uint8_t) (lba >> 8);
    command_table->command_fis.lba2 = (uint8_t) (lba >> 16);
    command_table->command_fis.lba3 = (uint8_t) (lba >> 24);
    command_table->command_fis.lba4 = (uint8_t) (lba >> 32);
    command_table->command_fis.lba5 = (uint8_t) (lba >> 48);
    command_table->command_fis.count_low = (uint8_t) count;
    command_table->command_fis.count_high = (uint8_t) (count >> 8);
    command_table->prdt->data_addr_low = (uint32_t) buf;
    command_table->prdt->description = (count * 512) - 1;
    command_hdr->flags |= sizeof(struct ahci_fis_h2d) / sizeof(uint32_t);
    if (ahci_64_bit) {
        command_table->prdt->data_addr_hi = 0;
    }
    command_hdr->command_table_low = (uint32_t) command_table;
    if (ahci_64_bit) {
        command_hdr->command_table_hi = 0;
    }
    command_hdr->prdt_count = 1;
    command_hdr->prdt_length = 8;
    if (write) {
        command_hdr->flags |= AHCI_CMD_HDR_FLAGS_W;
    } else {
        command_hdr->flags &= ~AHCI_CMD_HDR_FLAGS_W;
    }
    // issue the command
    while (abar->ports[port].task_file_data & (AHCI_PORT_TFD_STS_BSY | AHCI_PORT_TFD_STS_DRQ));
    abar->ports[port].command_issue |= (1 << slot);
    // Wait to complete and check for errors now
    while (abar->ports[port].command_issue & (1 << slot));
    if (abar->ports[port].interrupt_status & (1 << 30)) {
        free(command_table, sizeof(struct ahci_command_table));
        return -1;
    }
    free(command_table, sizeof(struct ahci_command_table));
    return 0;
}
