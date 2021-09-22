#include <stddef.h>
#include <hal/disk.h>
#include <tools/print.h>
#include <tools/string.h>

static struct disk_abstract floppy_inventory[MAX_FLOPPIES] = {0};
static struct disk_abstract disk_inventory[MAX_DISKS] = {0};
static int floppy_top = 0x00;
static int disk_top = 0x80;

static struct disk_abstract *get_disk(int disk) {
    if (disk < 0x80) {
        if (disk >= MAX_FLOPPIES) {
            return NULL;
        }
        return floppy_inventory[disk].present ? &floppy_inventory[disk] : NULL;
    }
    if ((disk - 0x80) >= MAX_DISKS) {
        return NULL;
    }
    return disk_inventory[disk - 0x80].present ? &disk_inventory[disk - 0x80] : NULL;
}

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

int hal_disk_submit(struct disk_abstract *disk, int flp) {
    print("HAL: Submitting a: %s", disk_type_to_name(disk->interface));
    if (flp) {
        if (floppy_top >= MAX_FLOPPIES) {
            print("HAL: Could not submit a \"%s\" because there aren't any free slots anymore", disk_type_to_name(disk->interface));
            return HAL_DISK_ENOMORE;
        }
        memcpy(&floppy_inventory[floppy_top], disk, sizeof(struct disk_abstract));
        floppy_inventory[floppy_top].present = 1;
        floppy_top++;
    } else {
        if ((disk_top - 0x80) >= MAX_DISKS) {
            print("HAL: Could not submit a \"%s\" because there aren't any free slots anymore", disk_type_to_name(disk->interface));
            return HAL_DISK_ENOMORE;
        }
        memcpy(&disk_inventory[disk_top - 0x80], disk, sizeof(struct disk_abstract));
        disk_inventory[disk_top - 0x80].present = 1;
        disk_top++;
    }
    return HAL_DISK_ESUCCESS;
}

int hal_disk_rw(int disk, void *buf, uint64_t lba, int len, int write) {
    struct disk_abstract *disk_abstract = get_disk(disk);
    if (!disk_abstract) {
        return HAL_DISK_ENOFOUND;
    }
    int (*rw)(struct disk_abstract *this, void *buf, uint64_t lba, int len, int write) =
        disk_abstract->ops.rw;
    if (!rw) {
        return HAL_DISK_ENOIMPL;
    }
    return rw(disk_abstract, buf, lba, len, write);
}
