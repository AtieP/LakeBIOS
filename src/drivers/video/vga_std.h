#ifndef __DRIVERS_VIDEO_VGA_STD_H__
#define __DRIVERS_VIDEO_VGA_STD_H__

#include <stdint.h>

void vga_regs_write(uint8_t misc, const uint8_t (*seq)[2], int seq_len, const uint8_t (*crtc)[2], int crtc_len, const uint8_t (*gfx)[2], int gfx_len, const uint8_t (*attr)[2], int attr_len);

#endif
