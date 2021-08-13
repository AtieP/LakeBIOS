#ifndef __HAL_DISPLAY_H__
#define __HAL_DISPLAY_H__

#include <stdint.h>

#define HAL_DISPLAY_VGA 0x01
#define HAL_DISPLAY_BGA 0x02
#define HAL_DISPLAY_VGA_BGA 0x03
#define HAL_DISPLAY_RAMFB 0x04
#define HAL_DISPLAY_VMWARE_VGA 0x05
struct display_abstract {
    int present;
    int interface;
    struct {
        int width;
        int height;
        int bpp;
        int pitch;
        void *buffer;
    } common;
    struct {
        int vga_mode;
        int text;
    } properties;
    struct {
        const void *font;
        int width;
        int height;
    } font;
    union {
        struct {
            void *fb;
            volatile uint8_t *bar2;
        } bga;
        struct {
            uint16_t bar0;
            uint32_t *fifo;
        } vmware_vga;
    } specific;
};

void hal_display_submit(struct display_abstract *display_abstract);
int hal_display_resolution(uint8_t display, int width, int height, int bpp, int clear, int text, int vga_mode);
int hal_display_font_get(uint8_t display, const void **font, int *width, int *height);
int hal_display_font_set(uint8_t display, const void *font, int width, int height);
int hal_display_plot_char(uint8_t display, int ch, int x, int y, uint8_t background_color, uint8_t foreground_color);
int hal_display_get_interface(uint8_t display);

#endif
