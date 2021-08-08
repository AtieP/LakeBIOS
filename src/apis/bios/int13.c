#include <apis/bios/bda.h>
#include <apis/bios/handlers.h>
#include <hal/disk.h>
#include <tools/print.h>

struct dap {
    uint8_t size;
    uint8_t reserved;
    uint16_t lba_count;
    uint16_t offset;
    uint16_t segment;
    uint64_t lba_start;
    // EDD 3.0+
    uint64_t ext_dest;
} __attribute__((__packed__));

struct edp {
    uint16_t size;
    uint16_t information;
    uint32_t cylinders;
    uint32_t heads_per_cylinder;
    uint32_t sectors_per_head;
    uint64_t lba_sectors;
    uint16_t bytes_per_sector;
    // 2.0+
    uint32_t edd_config_params;
} __attribute__((__packed__));

static uint64_t chs_to_lba(uint16_t cylinder, uint8_t head, uint8_t sector, uint8_t heads_per_cylinder, uint8_t sectors_per_head) {
    return ((cylinder * heads_per_cylinder + head) * sectors_per_head) + sector - 1;
}

static void handle_00(struct apis_bios_regs *regs) {
    if (regs->dl & (1 << 7)) {
        bda_write8(BDA_DISK_STATUS, 0x00);
    } else {
        bda_write8(BDA_FLOPPY_STATUS, 0x00);
    }
    regs->ah = 0x00;
    regs->eflags &= ~1;
}

static void handle_01(struct apis_bios_regs *regs) {
    if (regs->dl & (1 << 7)) {
        regs->ah = bda_read8(BDA_DISK_STATUS);
    } else {
        regs->ah = bda_read8(BDA_FLOPPY_STATUS);
    }
    if (regs->ah) {
        regs->eflags |= 1;
    } else {
        regs->eflags &= ~1;
    }
}

static void handle_02_03(struct apis_bios_regs *regs, int write) {
    uint8_t status;
    struct disk_abstract *disk_info = hal_disk_get(regs->dl);
    if (!disk_info) {
        status = 0x01;
        goto end;
    }
    uint16_t cylinder = regs->ch;
    uint8_t head = regs->dh;
    uint8_t sector = regs->cl & 0b111111;
    if (regs->dl & (1 << 7)) {
        cylinder |= ((uint16_t) (regs->cl & ~0b111111) << 2);
    }
    uint64_t lba = chs_to_lba(cylinder, head, sector, disk_info->common.heads_per_cylinder, disk_info->common.sectors_per_head);
    uint32_t destination = regs->es | regs->bx;
    if (hal_disk_rw(regs->dl, (void *) destination, lba, 512 * regs->al, write) != 0) {
        status = 0x01;
        regs->al = 0x00;
    } else {
        status = 0x00;
    }
end:
    if (regs->dl & (1 << 7)) {
        bda_write8(BDA_DISK_STATUS, status);
    } else {
        bda_write8(BDA_FLOPPY_STATUS, status);
    }
    regs->ah = status;
    if (status) {
        regs->eflags |= 1;
    } else {
        regs->eflags &= ~1;
    }
}

static void handle_41(struct apis_bios_regs *regs) {
    if (!(regs->dl & (1 << 7))) {
        bda_write8(BDA_FLOPPY_STATUS, 0x01);
        regs->ah = 0x01;
        regs->eflags |= 1;
        return;
    }
    if (regs->bx != 0x55aa) {
        bda_write8(BDA_DISK_STATUS, 0x01);
        regs->ah = 0x01;
        regs->eflags |= 1;
        return;
    }
    bda_write8(BDA_DISK_STATUS, 0x00);
    regs->bx = 0xaa55;
    regs->eflags &= ~1;
    regs->ah = 0x01;
    regs->cx = 1; // 42h, 43h, 44h, 47h, 48h
}

static void handle_42_43(struct apis_bios_regs *regs, int write) {
    uint8_t status;
    if (!(regs->dl & (1 << 7))) {
        bda_write8(BDA_FLOPPY_STATUS, 0x01);
        regs->ah = 0x01;
        regs->eflags |= 1;
        return;
    }
    struct disk_abstract *disk_info = hal_disk_get(regs->dl);
    if (!disk_info) {
        status = 0x01;
        goto finish;
    }
    struct dap *dap = (struct dap *) (regs->ds + regs->si);
    if (dap->size != 0x10) {
        status = 0x01;
        goto finish;
    }
    void *destination = (void *) (((uint32_t) dap->segment << 4) + dap->offset);
    if (hal_disk_rw(regs->dl, destination, dap->lba_start, dap->lba_count * 512, write) != 0) {
        status = 0x01;
        goto finish;
    }
    status = 0x00;
finish:
    bda_write8(BDA_DISK_STATUS, status);
    regs->ah = status;
    if (status) {
        regs->eflags |= 1;
    } else {
        regs->eflags &= ~1;
    }
}

static void handle_48(struct apis_bios_regs *regs) {
    if (!(regs->dl & (1 << 7))) {
        bda_write8(BDA_FLOPPY_STATUS, 0x01);
        regs->ah = 0x01;
        regs->eflags |= 1;
        return;
    }
    struct disk_abstract *disk_info = hal_disk_get(regs->dl);
    if (!disk_info) {
        bda_write8(BDA_DISK_STATUS, 0x01);
        regs->ah = 0x01;
        regs->eflags |= 1;
        return;
    }
    struct edp *edp = (struct edp *) (regs->ds + regs->si);
    edp->size = 0x1a;
    edp->information = (1 << 6) | 1; // CHS values are set to maximum, DMA boundary errors handled
    edp->cylinders = disk_info->common.lba_max / (disk_info->common.heads_per_cylinder * disk_info->common.sectors_per_head);
    edp->heads_per_cylinder = disk_info->common.heads_per_cylinder;
    edp->sectors_per_head = disk_info->common.sectors_per_head;
    edp->lba_sectors = disk_info->common.lba_max;
    edp->bytes_per_sector = 512;
    bda_write8(BDA_DISK_STATUS, 0x00);
    regs->ah = 0x00;
    regs->eflags &= ~1;
}

void apis_bios_int13(struct apis_bios_regs *regs) {
    uint8_t ah = regs->ah;
    if (ah == 0x00) {
        handle_00(regs);
    } else if (ah == 0x01) {
        handle_01(regs);
    } else if (ah == 0x02) {
        handle_02_03(regs, 0);
    } else if (ah == 0x03) {
        handle_02_03(regs, 1);
    } else if (ah == 0x41) {
        handle_41(regs);
    } else if (ah == 0x42) {
        handle_42_43(regs, 0);
    } else if (ah == 0x43) {
        handle_42_43(regs, 1);
    } else if (ah == 0x48) {
        handle_48(regs);
    } else {
        print("lakebios: bios: handler not available.");
        for (;;) {}
    }
}
