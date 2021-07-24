#ifndef __DRIVERS_VIDEO_VGA_IO_H__
#define __DRIVERS_VIDEO_VGA_IO_H__

#include <stdint.h>
#include <cpu/pio.h>

#define VGA_GFX_ADDR 0x3ce
#define VGA_GFX_DATA 0x3cf

#define VGA_GFX_SET_RESET 0x00
#define VGA_GFX_EN_SET_RESET 0x01
#define VGA_GFX_COLOR_CMP 0x02
#define VGA_GFX_DATA_ROTATE 0x03
#define VGA_GFX_READ_MAP_SEL 0x04
#define VGA_GFX_MODE 0x05
#define VGA_GFX_MISC 0x06
#define VGA_GFX_CDC 0x07 // color don't care
#define VGA_GFX_BIT_MASK 0x08

#define VGA_SEQ_ADDR 0x3c4
#define VGA_SEQ_DATA 0x3c5

#define VGA_SEQ_RESET 0x00
#define VGA_SEQ_CLOCKING 0x01
#define VGA_SEQ_MAP_MASK 0x02
#define VGA_SEQ_CHAR_MAP_SEL 0x03
#define VGA_SEQ_MEM_MODE 0x04

#define VGA_MISC_WRITE 0x3c2
#define VGA_MISC_READ 0x3cc

#define VGA_CRTC_ADDR 0x3d4
#define VGA_CRTC_DATA 0x3d5

#define VGA_CRTC_HORIZONTAL_TOTAL 0x00
#define VGA_CRTC_HORIZONTAL_DISPLAY_END 0x01
#define VGA_CRTC_HORIZONTAL_BLANK_START 0x02
#define VGA_CRTC_HORIZONTAL_BLANK_END 0x03
#define VGA_CRTC_HORIZONTAL_RETRACE_START 0x04
#define VGA_CRTC_HORIZONTAL_RETRACE_END 0x05
#define VGA_CRTC_VERTICAL_TOTAL 0x06
#define VGA_CRTC_OVERFLOW 0x07
#define VGA_CRTC_PRESET_ROW_SCAN 0x08
#define VGA_CRTC_MAXIMUM_SCAN_LINE 0x09
#define VGA_CRTC_CURSOR_START 0x0a
#define VGA_CRTC_CURSOR_END 0x0b
#define VGA_CRTC_START_ADDR_HI 0x0c
#define VGA_CRTC_START_ADDR_LO 0x0d
#define VGA_CRTC_CURSOR_LOC_HI 0x0e
#define VGA_CRTC_CURSOR_LOC_LO 0x0f
#define VGA_CRTC_VERTICAL_RETRACE_START 0x10
#define VGA_CRTC_VERTICAL_RETRACE_END 0x11
#define VGA_CRTC_VERTICAL_DISPLAY_END 0x12
#define VGA_CRTC_OFFSET 0x13
#define VGA_CRTC_UNDERLINE_LOCATION 0x14
#define VGA_CRTC_VERTICAL_BLANKING_START 0x15
#define VGA_CRTC_VERTICAL_BLANKING_END 0x16
#define VGA_CRTC_MODE_CONTROL 0x17
#define VGA_CRTC_LINE_COMPARE 0x18

#define VGA_AC_ADDR 0x3c0
#define VGA_AC_WRITE 0x3c0
#define VGA_AC_READ 0x3c1

#define VGA_AC_MODE_CONTROL 0x10
#define VGA_AC_OVERSCAN_COLOR 0x11
#define VGA_AC_COLOR_PLANE_ENABLE 0x12
#define VGA_AC_HORIZONTAL_PIXEL_PAN 0x13
#define VGA_AC_COLOR_SELECT 0x14

#define VGA_DAC_ADDR_READ 0x3c7
#define VGA_DAC_ADDR_WRITE 0x3c8
#define VGA_DAC_DATA 0x3c9

static inline uint8_t vga_gfx_read(uint8_t index) {
    outb(VGA_GFX_ADDR, index);
    return inb(VGA_GFX_DATA);
}

static inline void vga_gfx_write(uint8_t index, uint8_t value) {
    outb(VGA_GFX_ADDR, index);
    outb(VGA_GFX_DATA, value);
}

static inline uint8_t vga_seq_read(uint8_t index) {
    outb(VGA_SEQ_ADDR, index);
    return inb(VGA_SEQ_DATA);
}

static inline void vga_seq_write(uint8_t index, uint8_t value) {
    outb(VGA_SEQ_ADDR, index);
    outb(VGA_SEQ_DATA, value);
}

static inline uint8_t vga_crtc_read(uint8_t index) {
    outb(VGA_CRTC_ADDR, index);
    return inb(VGA_CRTC_DATA);
}

static inline void vga_crtc_write(uint8_t index, uint8_t value) {
    outb(VGA_CRTC_ADDR, index);
    outb(VGA_CRTC_DATA, value);
}

static inline uint8_t vga_attr_read(uint8_t index) {
    outb(VGA_AC_ADDR, index);
    return inb(VGA_AC_READ);
}

static inline void vga_attr_write(uint8_t index, uint8_t value) {
    outb(VGA_AC_ADDR, index);
    outb(VGA_AC_WRITE, value);
}

static inline uint8_t vga_misc_read() {
    return inb(VGA_MISC_READ);
}

static inline void vga_misc_write(uint8_t value) {
    outb(VGA_MISC_WRITE, value);
}

static inline void vga_dac_read(uint8_t index, uint8_t (*values)[3], int len) {
    outb(VGA_DAC_ADDR_READ, index);
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < 3; j++) {
            values[i][j] = inb(VGA_DAC_DATA);
        }
    }
}

static inline void vga_dac_write(uint8_t index, const uint8_t (*values)[3], int len) {
    outb(VGA_DAC_ADDR_WRITE, index);
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < 3; j++) {
            outb(VGA_DAC_DATA, values[i][j]);
        }
    }
}

#endif
