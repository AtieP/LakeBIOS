#include <hal/display.h>
#include <drivers/video/bochs_display.h>
#include <drivers/video/vga_modes.h>
#include <drivers/video/vmware_vga.h>
#include <paravirt/qemu.h>
#include <tools/string.h>
#include <tools/print.h>

// https://chrishewett.com/blog/true-rgb565-colour-picker/ :D

static struct display_abstract display_inventory[256] = {0};
static uint8_t display_inventory_index = 0x00;

static uint16_t gfx_pal_16[] = {
    0x0000,
    0x0014,
    0x0540,
    0x0555,
    0xa000,
    0xa014,
    0xa280,
    0xa554,
    0x52aa,
    0x52bf,
    0x57ea,
    0x57ff,
    0xfaaa,
    0xfafb,
    0xffea,
    0xffff
};

static uint32_t gfx_pal_24_32[] = {
    0x000000,
    0x0000aa,
    0x00aa00,
    0x00aaaa,
    0xaa0000,
    0xaa00aa,
    0xaa5500,
    0xaaaaaa,
    0x555555,
    0x5555ff,
    0x55ff55,
    0x55ffff,
    0xff5555,
    0xff55ff,
    0xffff55,
    0xffffff
};

static const char *display_type_to_name(int type) {
    switch (type) {
        case HAL_DISPLAY_VGA:
            return "VGA display";
        case HAL_DISPLAY_BGA:
            return "BGA display";
        case HAL_DISPLAY_VGA_BGA:
            return "VGA/BGA hybrid display";
        case HAL_DISPLAY_RAMFB:
            return "ramfb display";
        case HAL_DISPLAY_VMWARE_VGA:
            return "VMWare VGA display";
        default:
            return "Unknown display";
    }
}

void hal_display_submit(struct display_abstract *display_abstract) {
    print("HAL: Submitting a: %s", display_type_to_name(display_abstract->interface));
    memcpy(&display_inventory[display_inventory_index], display_abstract, sizeof(struct display_abstract));
    display_inventory[display_inventory_index].present = 1;
    display_inventory_index++;
}

int hal_display_resolution(uint8_t display, int width, int height, int bpp, int clear, int text, int vga_mode) {
    struct display_abstract *display_abstract = &display_inventory[display];
    if (!display_abstract->present) {
        return HAL_DISPLAY_ENOFOUND;
    }
    int (*resolution)(struct display_abstract *this, int width, int height, int bpp, int clear, int text, int vga_mode) = 
        display_abstract->ops.resolution;
    if (!resolution) {
        return HAL_DISPLAY_ENOIMPL;
    }
    return resolution(display_abstract, width, height, bpp, clear, text, vga_mode);
}

int hal_display_font_get(uint8_t display, const void **font, int *width, int *height) {
    struct display_abstract *display_abstract = &display_inventory[display];
    if (!display_abstract->present) {
        return HAL_DISPLAY_ENOFOUND;
    }
    int (*font_get)(struct display_abstract *this, const void **font, int *width, int *height) = 
        display_abstract->ops.font_get;
    if (!font_get) {
        return HAL_DISPLAY_ENOIMPL;
    }
    return font_get(display_abstract, font, width, height);
}

int hal_display_font_set(uint8_t display, const void *font, int width, int height) {
    struct display_abstract *display_abstract = &display_inventory[display];
    if (!display_abstract->present) {
        return HAL_DISPLAY_ENOFOUND;
    }
    int (*font_set)(struct display_abstract *this, const void *font, int width, int height) = 
        display_abstract->ops.font_set;
    if (!font_set) {
        return HAL_DISPLAY_ENOIMPL;
    }
    return font_set(display_abstract, font, width, height);
}

int hal_display_plot_char(uint8_t display, int ch, int x, int y, uint8_t background_color, uint8_t foreground_color) {
    struct display_abstract *display_abstract = &display_inventory[display];
    if (!display_abstract->present) {
        return HAL_DISPLAY_ENOIMPL;
    }
    int width = display_abstract->common.width;
    int height = display_abstract->common.height;
    if (display_abstract->properties.text) {
        width /= display_abstract->font.width;
        height /= display_abstract->font.height;
        int bpp = display_abstract->common.bpp;
        if (bpp == 4) {
            *((uint16_t *) display_abstract->common.buffer + (y * width + x)) = (((uint16_t) background_color << 12) | (uint16_t) foreground_color << 8) | ch;
        } else {
            return HAL_DISPLAY_ENOIMPL;
        }
    } else {
        int font_width = display_abstract->font.width;
        int font_height = display_abstract->font.height;
        uint8_t *glyph = &((uint8_t *) display_abstract->font.font)[ch * font_height];
        int pitch = display_abstract->common.pitch;
        int bpp = display_abstract->common.bpp;
        x *= font_width;
        y *= font_height;
        for (int i = 0; i < font_height; i++) {
            for (int j = font_width - 1; j >= 0; j--) {
                if (bpp == 8) {
                    *((uint8_t *) display_abstract->common.buffer + (y * (pitch / sizeof(uint8_t)) + x)) = glyph[i] & (1 << j) ? foreground_color : background_color;
                } else if (bpp == 16 || bpp == 15) {
                    *((uint16_t *) display_abstract->common.buffer + (y * (pitch / sizeof(uint16_t)) + x)) = glyph[i] & (1 << j) ? gfx_pal_16[foreground_color] : gfx_pal_16[background_color];
                } else if (bpp == 32) {
                    *((uint32_t *) display_abstract->common.buffer + (y * (pitch / sizeof(uint32_t)) + x)) = glyph[i] & (1 << j) ? gfx_pal_24_32[foreground_color] : gfx_pal_24_32[background_color];
                }
                x++;
            }
            x -= font_width;
            y++;
        }
    }
    return HAL_DISPLAY_ESUCCESS;
}

int hal_display_get_interface(uint8_t display) {
    return display_inventory[display].present ? display_inventory[display].interface : HAL_DISPLAY_ENOFOUND;
}
