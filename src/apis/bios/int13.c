#include <apis/bios/bda.h>
#include <apis/bios/handlers.h>
#include <hal/disk.h>
#include <tools/print.h>

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
    } else {
        print("lakebios: bios: handler not available.");
        for (;;) {}
    }
}
