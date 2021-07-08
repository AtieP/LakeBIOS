#include <hal.h>
#include <stddef.h>
#include <tools/alloc.h>
#include <tools/print.h>
#include <tools/string.h>

static struct disk_abstract disk_inventory[256] = {0};
static size_t disk_bios_dl = 0;

static const char *type_to_name(int type) {
    switch (type) {
        case HAL_DISK_AHCI:
            return "n AHCI disk";
        case HAL_DISK_NVME:
            return " NVME disk";
        case HAL_DISK_FLP:
            return " floppy disk";
        default:
            return "n unknown disk";
    }
}

void hal_disk_submit(struct disk_abstract *disk) {
    print("lakebios: HAL: submitting a%s", type_to_name(disk->interface));
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
        return ahci_command(
            disk_inventory[i].specific.ahci.abar,
            disk_inventory[i].specific.ahci.port,
            write ? 0x35 : 0x25, buf, lba, len, write ? 1 : 0,
            disk_inventory[i].specific.ahci.atapi
        );
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
        return -1; // Unsupported for now
    }
    return -1;
}
