#ifndef __DRIVERS_VIDEO_VGA_PALLETES_H__
#define __DRIVERS_VIDEO_VGA_PALLETES_H__

#include <stdint.h>

#define vga_pallete_16_len (16 * 4)
#define vga_pallete_256_len (64 * 4)
// #define vga_pallete_256_len (256 * 3)

extern const uint8_t vga_pallete_16[][3];
extern const uint8_t vga_pallete_256[][3];

#endif

