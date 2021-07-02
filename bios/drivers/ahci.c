#include <cpu/pio.h>
#include <drivers/pci.h>
#include <drivers/ahci.h>
#include <tools/alloc.h>
#include <tools/print.h>

static volatile struct ahci_abar *abar;
static int ahci_available_ports;
static int ahci_availabke_cmd_slots;
static int ahci_staggered_spin_up;
static uint8_t ahci_bus;
static uint8_t ahci_slot;
static uint8_t ahci_function;

int ahci_detect() {
    return pci_get_device(AHCI_CLASS, AHCI_SUBCLASS, AHCI_INTERFACE, &ahci_bus, &ahci_slot, &ahci_function);
}

int ahci_setup() {
    if (pci_get_device(AHCI_CLASS, AHCI_SUBCLASS, AHCI_INTERFACE, &ahci_bus, &ahci_slot, &ahci_function) != 0) {
        return -1;
    }
    abar = (struct ahci_abar *) (uint32_t) pci_get_bar(ahci_bus, ahci_slot, ahci_function, 5);
    pci_enable_bus_mastering(ahci_bus, ahci_slot, ahci_function);
    // Gather important information
    ahci_available_ports = (abar->ghc.hba_capabilities & AHCI_CAP_PORTS_MASK) + 1;
    ahci_availabke_cmd_slots = ((abar->ghc.hba_capabilities >> 8) & AHCI_CAP_SLOTS_MASK) + 1;
    if (abar->ghc.hba_capabilities & AHCI_CAP_SSS) {
        print("atiebios: AHCI: staggered spin up supported");
        ahci_staggered_spin_up = 1;
    } else {
        ahci_staggered_spin_up = 0;
    }
    print("atiebios: AHCI: available ports, slots: %d, %d", ahci_available_ports, ahci_availabke_cmd_slots);
    // Enable AHCI mode again
    abar->ghc.ccc_control |= AHCI_GHC_CNT_AE;
    // Indicate BIOS ownership over AHCI if possible
    if (abar->ghc.hba_capabilities_ext & AHCI_GHC_CAP_EXT_BIOS) {
        print("atiebios: AHCI: taking ownership over the controller");
        abar->ghc.bios_handoff_cnt_sts |= AHCI_GHC_BIOS_BOS;
    }
    // Make sure all ports are idle
    for (int i = 0; i < ahci_available_ports; i++) {
        while (1) {
            uint32_t command_status = abar->ports[i].command_status;
            if (command_status & (AHCI_PORT_CMD_STS_ST | AHCI_PORT_CMD_STS_FRE | AHCI_PORT_CMD_STS_FR | AHCI_PORT_CMD_STS_CR)) {
                continue;  
            }
            break;
        }
    }
    // Reset the controller
    abar->ghc.global_hba_control |= AHCI_GHC_CNT_RESET;
    while (abar->ghc.global_hba_control & AHCI_GHC_CNT_RESET);
    // Enable AHCI mode again
    abar->ghc.ccc_control |= AHCI_GHC_CNT_AE;
    // Set up ports
    for (int i = 0; i < ahci_available_ports; i++) {
        // Set a command list
        uint32_t commands_list_addr_low = (uint32_t) calloc(sizeof(struct ahci_command_hdr) * ahci_availabke_cmd_slots, 1024);
        if (!commands_list_addr_low) {
            print("atiebios: AHCI: could not allocate command list for port %d, aborting", i);
            return -1;
        }
        abar->ports[i].commands_list_addr_low = commands_list_addr_low;
        abar->ports[i].commands_list_addr_hi = 0;
        // Set a receive FIS area
        uint32_t receive_fis = (uint32_t) calloc(sizeof(struct ahci_fis_hba), 256);
        if (!receive_fis) {
            print("atiebios: AHCI: could not allocate receive FIS for port %d, aborting", i);
        }
        abar->ports[i].fis_addr_low = receive_fis;
        abar->ports[i].fis_addr_hi = 0;
        // Enable FIS receives
        abar->ports[i].command_status |= AHCI_PORT_CMD_STS_FRE;
        // Staggered spin up
        if (ahci_staggered_spin_up) {
            abar->ports[i].command_status |= AHCI_PORT_CMD_STS_SUD;
        }
        // Wait 1ms
        for (volatile int i = 0; i < 1000; i++) {
            inb(0x80);
        }
        uint8_t det = abar->ports[i].sata_status & AHCI_PORT_SATA_STS_DET_MASK;
        if (det == 0) {
            print("atiebios: AHCI: no device detected and no communication established in port %d", i);
        } else if (det == 1) {
            print("atiebios: AHCI: device detected but no communication established in port %d", i);
        } else if (det == 3) {
            print("atiebios: AHCI: device detected and connection established in port %d!", i);
        } else if (det == 4) {
            print("atiebios: AHCI: no connection established because the interface is disabled or running in BIST loopback mode in port %d", i);
        }
        // Clear error
        abar->ports[i].sata_error = 0xffffffff;
        // Wait
        while (1) {
            uint32_t tfd = abar->ports[i].task_file_data;
            if (tfd & (AHCI_PORT_TFD_STS_BSY | AHCI_PORT_TFD_STS_DRQ)) {
                continue;
            }
            break;
        }
    }
    return 0;
}
