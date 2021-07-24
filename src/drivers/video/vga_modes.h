#ifndef __DRIVERS_VIDEO_VGA_MODES_H__
#define __DRIVERS_VIDEO_VGA_MODES_H__

#include <stdint.h>

struct vga_mode {
    uint8_t misc;
    const uint8_t (*seq)[2];
    int seq_len;
    const uint8_t (*crtc)[2];
    int crtc_len;
    const uint8_t (*gfx)[2];
    int gfx_len;
    const uint8_t (*attr)[2];
    int attr_len;
};

extern struct vga_mode vga_mode_80x25_text;
extern struct vga_mode vga_mode_320x200x256_linear;
#endif
