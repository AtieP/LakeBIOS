#ifndef __HAL_DISK_H__
#define __HAL_DISK_H__

#include <stdint.h>
#include <drivers/ahci.h>
#include <drivers/nvme.h>

#define HAL_DISK_AHCI 0x01
#define HAL_DISK_NVME 0x02
#define HAL_DISK_FLP  0x03
#define HAL_DISK_ATA_PIO 0x04
struct disk_abstract {
    int present;
    int interface;
    struct {
        uint64_t lba_max;
    } common;
    union {
        struct {
            volatile struct ahci_abar *abar;
            int port;
            int atapi;
            int drive;
            int lba48;
        } ahci;
        struct {
            volatile struct nvme_configuration *cfg; 
            volatile struct nvme_submission_entry *sq;
            volatile struct nvme_completion_entry *cq;
            int queue_id;
            uint32_t tail;
            uint32_t head;
            int namespace_id;
        } nvme;
        struct {
            uint16_t io_base;
            uint8_t kind;
        } floppy;
        struct {
            uint16_t io_base;
            int drive;
        } ata_pio;
    } specific;
};

void hal_disk_submit(struct disk_abstract *disk, int flp);
int hal_disk_rw(uint8_t bios_dl, void *buf, uint64_t lba, int len, int write);

#endif
