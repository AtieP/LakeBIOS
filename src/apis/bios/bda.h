#ifndef __APIS_BIOS_BDA_H__
#define __APIS_BIOS_BDA_H__

#include <stdint.h>

#define APIS_BIOS_BDA 0x400

#define bda_write8(offset, value) (*((volatile uint8_t *) (APIS_BIOS_BDA + offset)) = value)
#define bda_read8(offset) (*((volatile uint8_t *) (APIS_BIOS_BDA + offset)))
#define bda_write16(offset, value) (*((volatile uint16_t *) (APIS_BIOS_BDA + offset)) = value)
#define bda_read16(offset) (*((volatile uint16_t *) (APIS_BIOS_BDA + offset)))

#define BDA_FLOPPY_STATUS 0x41
#define BDA_VIDEO_MODE 0x49
#define BDA_SCREEN_COLUMNS 0x4a
#define BDA_CURSOR_END_SCAN 0x60
#define BDA_CURSOR_START_SCAN 0x61
#define BDA_ACTIVE_PAGE 0x62
#define BDA_CURSOR_POSITION (page) (0x63 + (page * 2))
#define BDA_DISK_STATUS 0x74

#endif
