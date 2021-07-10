#ifndef __DRIVERS_ATA_COMMON_H__
#define __DRIVERS_ATA_COMMON_H__

#include <stdint.h>

#define ATA_DRIVE_MASTER 0xa0
#define ATA_DRIVE_SLAVE 0xb0

#define ATA_COMMAND_READ 0x24
#define ATA_COMMAND_READ_DMA_EXT 0x25
#define ATA_COMMAND_WRITE 0x34
#define ATA_COMMAND_WRITE_DMA_EXT 0x35
#define ATA_COMMAND_IDENTIFY 0xec

int ata_common_identify_is_lba48(const uint16_t *identify);
uint64_t ata_common_identify_sectors(const uint16_t *identify, int lba48);

#endif
