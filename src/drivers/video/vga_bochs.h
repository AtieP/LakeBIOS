#ifndef __DRIVERS_VIDEO_VGA_BOCHS_H__
#define __DRIVERD_VIDEO_VGA_BOCHS_H__

#include <stdint.h>

#define BGA_ADDR 0x1ce
#define BGA_DATA 0x1cf

#define BDA_ID 0x00
#define BGA_X 0x01
#define BGA_Y 0x02
#define BGA_BPP 0x03
#define BGA_ENABLE 0x04

int bga_detect();
void *bga_get_fb();
void bga_resolution(int x, int y, int bpp, int clear);
void bga_deinit();

#endif
