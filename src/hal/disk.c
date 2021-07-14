#include <stddef.h>
#include <drivers/ata_common.h>
#include <hal/disk.h>
#include <tools/alloc.h>
#include <tools/print.h>
#include <tools/string.h>

static struct disk_abstract disk_inventory[256] = {0};
static size_t disk_bios_dl = 0;

static const char *disk_type_to_name(int type) {
    switch (type) {
        case HAL_DISK_AHCI:
            return "AHCI disk";
        case HAL_DISK_NVME:
            return "NVME disk";
        case HAL_DISK_FLP:
            return "Floppy disk";
        default:
            return "Unknown disk";
    }
}

void hal_disk_submit(struct disk_abstract *disk) {
    print("lakebios: HAL: submitting a: %s", disk_type_to_name(disk->interface));
    memcpy(&disk_inventory[disk_bios_dl], disk, sizeof(struct disk_abstract));
    disk_inventory[disk_bios_dl].present = 1;
    disk_bios_dl++;
}

int hal_disk_rw(uint8_t bios_dl, void *buf, uint64_t lba, int len, int write) {
    size_t i = bios_dl;
    if (!disk_inventory[i].present) {
        return -1;
    }
    if (disk_inventory[i].interface == HAL_DISK_AHCI) {
        if (disk_inventory[i].specific.ahci.atapi) {
            return -1;
        }
        if (!disk_inventory[i].specific.ahci.lba48) {
            // If drive is not lba48, do not access more than 2^28 sectors
            if ((lba + (len / 512)) & ~0x1fffffff) {
                return -1;
            }
        }
        if (disk_inventory[i].common.lba_max < (lba + (len / 512))) {
            return -1; // Out of bounds
        }
        // TODO: calculate the amount of PRDTs
        struct ahci_command_tbl *tbl = calloc(sizeof(struct ahci_command_tbl) + sizeof(struct ahci_prdt), 128);
        if (!tbl) {
            return -1; // Allocation error
        }
        tbl->command_fis.fis_kind = AHCI_FIS_H2D;
        tbl->command_fis.flags = 1 << 7;
        tbl->command_fis.device = disk_inventory[i].specific.ahci.drive | (1 << 6); // LBA addressing
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
            disk_inventory[i].specific.ahci.abar,
            disk_inventory[i].specific.ahci.port,
            write,
            disk_inventory[i].specific.ahci.atapi,
            tbl,
            1
        );
        free(tbl, sizeof(struct ahci_command_tbl) + sizeof(struct ahci_prdt));
        return ret;
    }
    if (disk_inventory[i].interface == HAL_DISK_NVME) {
        struct nvme_submission_entry cmd;
        cmd.opcode = write ? 0x01 : 0x02;
        cmd.namespace_id = disk_inventory[i].specific.nvme.namespace_id;
        cmd.prp1 = (uint64_t) (uintptr_t) buf;
        cmd.cmd_specific[0] = (uint32_t) lba;
        cmd.cmd_specific[1] = (uint32_t) (lba >> 32);
        cmd.cmd_specific[2] = (uint16_t) (len / 512) - 1;
        return nvme_command(
            disk_inventory[i].specific.nvme.cfg,
            &cmd,
            disk_inventory[i].specific.nvme.sq,
            disk_inventory[i].specific.nvme.cq,
            disk_inventory[i].specific.nvme.queue_id,
            &disk_inventory[i].specific.nvme.tail,
            &disk_inventory[i].specific.nvme.head
        );
    }
    if (disk_inventory[i].interface == HAL_DISK_FLP) {
        return -1; // Unsupported
    }
    return -1;
}