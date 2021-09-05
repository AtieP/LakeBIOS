#include <cpu/misc.h>
#include <cpu/pio.h>
#include <drivers/bus/pci.h>
#include <drivers/storage/ahci.h>
#include <drivers/storage/ata_common.h>
#include <hal/disk.h>
#include <tools/alloc.h>
#include <tools/print.h>
#include <tools/string.h>

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
        print("AHCI: Could not allocate command list for port %d", port);
        return -1;
    }
    uint32_t receive_fis = (uint32_t) calloc(sizeof(struct ahci_fis_hba), 256);
    if (!command_list) {
        free((void *) command_list, sizeof(struct ahci_command_hdr) * get_slots(abar));
        print("AHCI: Could not allocate receive FIS for port %d", port);
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
    if (port->commands_list_addr_low) {
        free((void *) port->commands_list_addr_low, sizeof(struct ahci_command_hdr) * get_slots(abar));
    }
    if (port->fis_addr_low) {
        free((void *) port->fis_addr_low, sizeof(struct ahci_fis_hba));
    }
}

static void port_deinit(volatile struct ahci_abar *abar, int index) {
    // Stop execution
    volatile struct ahci_port *port = (volatile struct ahci_port *) &abar->ports[index];
    port->command_status &= ~(AHCI_PORT_CMD_STS_FRE | AHCI_PORT_CMD_STS_ST);
    while (port->command_status & (AHCI_PORT_CMD_STS_CR | AHCI_PORT_CMD_STS_FR)) {
        pause();
    }
    // No more interrupts
    port->interrupt_enable = 0;
    port->interrupt_status = 0xffffffff;
    // Free all memory allocated by it
    port_free(abar, index);
}

static int hal_submit(struct disk_abstract *disk, int flp);

static int port_init(volatile struct ahci_abar *abar, int index) {
    volatile struct ahci_port *port = (volatile struct ahci_port *) &abar->ports[index];
    port->command_status &= ~(AHCI_PORT_CMD_STS_FRE | AHCI_PORT_CMD_STS_ST);
    while (port->command_status & (AHCI_PORT_CMD_STS_CR | AHCI_PORT_CMD_STS_FR)) {
        pause();
    }
    port->interrupt_enable = 0;
    port->interrupt_status = 0xffffffff;
    if (port_alloc(abar, index) != 0) {
        return -1;
    }
    if (sss_supported(abar)) {
        // Staggered spinup
        port->command_status |= AHCI_PORT_CMD_STS_SUD;
    }
    // Device must be brought up
    if ((port->sata_status & AHCI_PORT_SATA_STS_DET_MASK) != 3) {
        port_deinit(abar, index);
        return -1;
    }
    port->command_status |= AHCI_PORT_CMD_STS_FRE; // Otherwise, the status bits get stuck
    // Clear errors and wait the device for being ready
    port->sata_error |= port->sata_error;
    if (port->task_file_data & (AHCI_PORT_TFD_STS_BSY | AHCI_PORT_TFD_STS_DRQ)) {
        port_deinit(abar, index);
        return -1;
    }
    // Execute commands
    port->command_status |= AHCI_PORT_CMD_STS_ST;
    // Identify
    struct ahci_command_tbl *tbl = calloc(sizeof(struct ahci_command_tbl) + sizeof(struct ahci_prdt), 128);
    if (!tbl) {
        port_deinit(abar, index);
        return -1;
    }
    // The identify buffer is 512 bytes in size
    uint16_t *identify_buffer = calloc(512, 1);
    if (!identify_buffer) {
        free(tbl, sizeof(struct ahci_command_tbl) + sizeof(struct ahci_prdt));
        port_deinit(abar, index);
        return -1;
    }
    tbl->command_fis.fis_kind = AHCI_FIS_H2D;
    tbl->command_fis.command = ATA_COMMAND_IDENTIFY;
    tbl->command_fis.flags = 1 << 7;
    tbl->prdt[0].data_addr_low = (uint32_t) identify_buffer;
    tbl->prdt[0].description = 512 - 1;
    if (ahci_command(abar, index, 0, 0, tbl, 1) == -1) {
        port_deinit(abar, index);
        return -1;
    }
    free(identify_buffer, 512);
    // Submit to the HAL
    int lba48 = ata_common_identify_is_lba48(identify_buffer);
    struct disk_abstract disk = {0};
    disk.interface = HAL_DISK_AHCI;
    disk.common.lba_max = ata_common_identify_sectors(identify_buffer, lba48);
    disk.common.heads_per_cylinder = 16;
    disk.common.sectors_per_head = 255;
    disk.specific.ahci.abar = abar;
    disk.specific.ahci.atapi = 0;
    disk.specific.ahci.port = index;
    disk.specific.ahci.lba48 = lba48;
    disk.specific.ahci.drive = ATA_DRIVE_MASTER;
    if (hal_submit(&disk, 0) == HAL_DISK_ENOMORE) {
        port_deinit(abar, index);
        return -1;
    }
    return 0;
}

static int controller_init(uint8_t ahci_bus, uint8_t ahci_slot, uint8_t ahci_function) {
    volatile struct ahci_abar *abar = (volatile struct ahci_abar *) (uintptr_t) pci_get_bar(ahci_bus, ahci_slot, ahci_function, 5);
    if (!abar) {
        return -1; // Very unlikely though
    }
    pci_control_set(ahci_bus, ahci_slot, ahci_function, PCI_CFG_COMMAND_BUS_MASTER | PCI_CFG_COMMAND_IO_ENABLE | PCI_CFG_COMMAND_MEM_ENABLE);
    abar->ghc.global_hba_control |= AHCI_GHC_CNT_AE;
    // Reset
    abar->ghc.global_hba_control |= AHCI_GHC_CNT_RESET;
    while (abar->ghc.global_hba_control & AHCI_GHC_CNT_RESET) {
        pause();
    }
    for (int i = 0; i < get_ports_silicon(abar); i++) {
        if (port_implemented(abar, i)) {
            if (port_init(abar, i) == 0) {
                print("AHCI: port %d initialized successfully", i);
            } else {
                print("AHCI: port %d could not be initialized", i);
            }
        }
    }
    return 0;
}

void ahci_init() {
    print("AHCI: Initializing controllers");
    size_t i;
    for (i = 0; i < SIZE_MAX; i++) {
        uint8_t ahci_bus;
        uint8_t ahci_slot;
        uint8_t ahci_function;
        if (pci_get_device(AHCI_CLASS, AHCI_SUBCLASS, AHCI_INTERFACE, &ahci_bus, &ahci_slot, &ahci_function, i) == 0) {
            print("AHCI: Controller found at PCI Bus %d Slot %d Function %d", ahci_bus, ahci_slot, ahci_function);
            if (controller_init(ahci_bus, ahci_slot, ahci_function) == -1) {
                print("AHCI: Controller mentioned before has not been initialized successfully");
            } else {
                print("AHCI: Controller mentioned before has been initialized successfully");
            }
        } else {
            break;
        }
    }
    print("AHCI: Finished initializing controllers");
}

int ahci_command(volatile struct ahci_abar *abar, int port, int write, int atapi, struct ahci_command_tbl *tbl, int prdt_len) {
    if (atapi) {
        return -1;
    }
    int slot = get_free_slot(abar, port);
    if (slot == -1) {
        return -1;
    }
    volatile struct ahci_command_hdr *hdr = (volatile struct ahci_command_hdr *) abar->ports[port].commands_list_addr_low;
    hdr += slot;
    memset((void *) hdr, 0, sizeof(struct ahci_command_hdr));
    hdr->flags =
          (prdt_len << AHCI_CMD_HDR_FLAGS_PRDTL_SHIFT)
        | (1 << 10)
        | (write ? AHCI_CMD_HDR_FLAGS_W : 0) 
        | (atapi ? AHCI_CMD_HDR_FLAGS_ATAPI : 0)
        | (sizeof(struct ahci_fis_h2d) / 4)
    ;
    hdr->command_table_low = (uint32_t) tbl;
    abar->ports[port].interrupt_status = 0xffffffff;
    // Wait, issue, check
    while (abar->ports[port].task_file_data & (AHCI_PORT_TFD_STS_BSY | AHCI_PORT_TFD_STS_DRQ)) {
        pause();
    }
    abar->ports[port].command_issue |= (1 << slot);
    while (abar->ports[port].command_issue & (1 << slot)) {
        pause();
    }
    if (abar->ports[port].interrupt_status & (1 << 30)) {
        return -1;
    } else {
        return 0;
    }
}

/* HAL Functions */

static int hal_rw(struct disk_abstract *this, void *buf, uint64_t lba, int len, int write) {
    // TODO: PRDTs
    struct ahci_command_tbl *tbl = calloc(sizeof(struct ahci_command_tbl) + sizeof(struct ahci_prdt), 128);
    if (!tbl) {
        return HAL_DISK_ENOMEM;
    }
    tbl->command_fis.fis_kind = AHCI_FIS_H2D;
    tbl->command_fis.flags = 1 << 7;
    tbl->command_fis.device = this->specific.ahci.drive | (1 << 6); // LBA addressing
    tbl->command_fis.command = write ? ATA_COMMAND_WRITE_DMA_EXT : ATA_COMMAND_READ_DMA_EXT;
    tbl->command_fis.lba0 = (uint8_t) (lba);
    tbl->command_fis.lba1 = (uint8_t) (lba >> 8);
    tbl->command_fis.lba2 = (uint8_t) (lba >> 16);
    tbl->command_fis.lba3 = (uint8_t) (lba >> 24);
    tbl->command_fis.lba4 = (uint8_t) (lba >> 32);
    tbl->command_fis.lba5 = (uint8_t) (lba >> 40);
    tbl->command_fis.count_low = (uint8_t) (len / 512);
    tbl->command_fis.count_hi = (uint8_t) ((len / 512) >> 8);
    tbl->prdt[0].data_addr_low = (uint32_t) buf;
    tbl->prdt[0].description = len - 1;
    int ret = ahci_command(
        this->specific.ahci.abar,
        this->specific.ahci.port,
        write,
        this->specific.ahci.atapi,
        tbl,
        1
    );
    free(tbl, sizeof(struct ahci_command_tbl) + sizeof(struct ahci_prdt));
    return ret != 0 ? HAL_DISK_EUNK : HAL_DISK_ESUCCESS;
}

static int hal_submit(struct disk_abstract *disk, int flp) {
    disk->ops.rw = hal_rw;
    return hal_disk_submit(disk, flp);
}
