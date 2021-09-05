#ifndef __DRIVERS_VIDEO_BOCHS_DISPLAY_H__
#define __DRIVERS_VIDEO_BOCHS_DISPLAY_H__

#include <stdint.h>
#include <drivers/video/vga_modes.h>

#define BOCHS_DISPLAY_VENDOR 0x1234
#define BOCHS_DISPLAY_DEVICE 0x1111

#define BOCHS_DISPI_ID   0x00
#define BOCHS_DISPI_XRES 0x01
#define BOCHS_DISPI_YRES 0x02
#define BOCHS_DISPI_BPP  0x03
#define BOCHS_DISPI_EN   0x04
#define BOCHS_DISPI_BANK 0x05

void bochs_display_init();

/* VGA compatible functions */
void bochs_display_vga_regs_write(volatile uint8_t *bar2, struct vga_mode *mode);
void bochs_display_vga_font_write(volatile uint8_t *bar2, const void *font, int height);
void bochs_display_vga_font_read(volatile uint8_t *bar2, void *font, int height);

/* bochs-display functions */
void bochs_display_high_res(volatile uint8_t *bar2, int width, int height, int bpp, int clear);

#endif
