#include <stddef.h>
#include <drivers/bus/pci.h>
#include <drivers/video/bochs_display.h>
#include <drivers/video/vga_regs.h>
#include <hal/display.h>
#include <tools/print.h>
#include <tools/string.h>

#define VGA_REG_OFFSET(reg) (0x400 + ((reg) - 0x3c0))
#define VBE_REG_OFFSET(reg) (0x500 + (reg))

/* Read/Write from BAR2 */

static inline uint8_t reg_read_byte(volatile uint8_t *bar2, size_t offset) {
    return *(bar2 + offset);
}

static inline void reg_write_byte(volatile uint8_t *bar2, size_t offset, uint8_t value) {
    *(bar2 + offset) = value;
}

static inline uint16_t reg_read_word(volatile uint8_t *bar2, size_t offset) {
    return *((uint16_t *) (bar2 + offset));
}

static inline void reg_write_word(volatile uint8_t *bar2, size_t offset, uint16_t value) {
    *((uint16_t *) (bar2 + offset)) = value;
}

/* Read/Write from VGA registers */

static uint8_t gfx_read(volatile uint8_t *bar2, uint8_t index) {
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_GFX_ADDR), index);
    return reg_read_byte(bar2, VGA_REG_OFFSET(VGA_GFX_DATA));
}

static void gfx_write(volatile uint8_t *bar2, uint8_t index, uint8_t value) {
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_GFX_ADDR), index);
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_GFX_DATA), value);
}

static uint8_t seq_read(volatile uint8_t *bar2, uint8_t index) {
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_SEQ_ADDR), index);
    return reg_read_byte(bar2, VGA_REG_OFFSET(VGA_SEQ_DATA));
}

static void seq_write(volatile uint8_t *bar2, uint8_t index, uint8_t data) {
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_SEQ_ADDR), index);
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_SEQ_DATA), data);
}

static uint8_t crtc_read(volatile uint8_t *bar2, uint8_t index) {
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_CRTC_ADDR), index);
    return reg_read_byte(bar2, VGA_REG_OFFSET(VGA_CRTC_DATA));
}

static void crtc_write(volatile uint8_t *bar2, uint8_t index, uint8_t value) {
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_CRTC_ADDR), index);
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_CRTC_DATA), value);
}

static uint8_t attr_read(volatile uint8_t *bar2, uint8_t index) {
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_AC_ADDR), index);
    return reg_read_byte(bar2, VGA_REG_OFFSET(VGA_AC_READ));
}

static void attr_write(volatile uint8_t *bar2, uint8_t index, uint8_t value) {
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_AC_ADDR), index);
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_AC_ADDR), value);
}

static uint8_t misc_read(volatile uint8_t *bar2) {
    return reg_read_byte(bar2, VGA_REG_OFFSET(VGA_MISC_READ));
}

static void misc_write(volatile uint8_t *bar2, uint8_t value) {
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_MISC_WRITE), value);
}

static void dac_write(volatile uint8_t *bar2, uint8_t index, const uint8_t (*values)[3], int len) {
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_DAC_ADDR_WRITE), index);
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < 3; j++) {
            reg_write_byte(bar2, VGA_REG_OFFSET(VGA_DAC_DATA), values[i][j]);
        }
    }
}

static void dac_read(volatile uint8_t *bar2, uint8_t index, uint8_t (*values)[3], int len) {
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_DAC_ADDR_READ), index);
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < 3; j++) {
            values[i][j] = reg_read_byte(bar2, VGA_REG_OFFSET(VGA_DAC_DATA));
        }
    }
}

/* Read/Write from VBE registers */
static uint16_t vbe_read(volatile uint8_t *bar2, uint8_t index) {
    return reg_read_word(bar2, VBE_REG_OFFSET(index << 1));
}

static void vbe_write(volatile uint8_t *bar2, uint8_t index, uint16_t value) {
    reg_write_word(bar2, VBE_REG_OFFSET(index << 1), value);
}

/* Others */
static void vga_font_access(volatile uint8_t *bar2, int unlock) {
    static uint8_t seq_reset = 0;
    static uint8_t seq_map_mask = 0;
    static uint8_t seq_mem_mode = 0;
    static uint8_t gfx_read_map = 0;
    static uint8_t gfx_mode = 0;
    static uint8_t gfx_misc = 0;
    if (unlock) {
        seq_reset = seq_read(bar2, VGA_SEQ_RESET);
        seq_map_mask = seq_read(bar2, VGA_SEQ_MAP_MASK);
        seq_mem_mode = seq_read(bar2, VGA_SEQ_MEM_MODE);
        gfx_read_map = gfx_read(bar2, VGA_GFX_READ_MAP_SEL);
        gfx_mode = gfx_read(bar2, VGA_GFX_MODE);
        gfx_misc = gfx_read(bar2, VGA_GFX_MISC);
        seq_write(bar2, VGA_SEQ_RESET, 0x01);
        seq_write(bar2, VGA_SEQ_MAP_MASK, 0x04);
        seq_write(bar2, VGA_SEQ_MEM_MODE, 0x06);
        seq_write(bar2, VGA_SEQ_RESET, 0x03);
        gfx_write(bar2, VGA_GFX_READ_MAP_SEL, 0x02);
        gfx_write(bar2, VGA_GFX_MODE, 0x00);
        gfx_write(bar2, VGA_GFX_MISC, 0x04);
    } else {
        seq_write(bar2, VGA_SEQ_RESET, seq_reset);
        seq_write(bar2, VGA_SEQ_MAP_MASK, seq_map_mask);
        seq_write(bar2, VGA_SEQ_MEM_MODE, seq_mem_mode);
        gfx_write(bar2, VGA_GFX_READ_MAP_SEL, gfx_read_map);
        gfx_write(bar2, VGA_GFX_MODE, gfx_mode);
        gfx_write(bar2, VGA_GFX_MISC, gfx_misc);
    }
}

/* Initialization functions */

static void hal_submit(struct display_abstract *display);

static void vga_compat_controller_init(uint8_t bus, uint8_t slot, uint8_t function) {
    volatile uint8_t *bar2 = (uint8_t *) (uintptr_t) pci_get_bar(bus, slot, function, 2);
    misc_write(bar2, 0xc3);
    seq_write(bar2, 0x04, 0x02);
    vbe_write(bar2, BOCHS_DISPI_EN, 0x00);
    gfx_write(bar2, VGA_GFX_SET_RESET, 0x00);
    gfx_write(bar2, VGA_GFX_EN_SET_RESET, 0x00);
    seq_write(bar2, VGA_SEQ_CLOCKING, seq_read(bar2, VGA_SEQ_CLOCKING) | (1 << 5));
    seq_write(bar2, VGA_SEQ_MAP_MASK, 0x00);
    misc_write(bar2, misc_read(bar2) & ~(1 << 1));
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_AC_ADDR), 0x00);
    struct display_abstract display;
    display.interface = HAL_DISPLAY_VGA_BGA;
    display.specific.bga.bar2 = bar2;
    display.specific.bga.fb = (void *) (uintptr_t) pci_get_bar(bus, slot, function, 0);
    hal_submit(&display);
}

static void non_vga_compat_controller_init(uint8_t bus, uint8_t slot, uint8_t function) {
    volatile uint8_t *bar2 = (uint8_t *) (uintptr_t) pci_get_bar(bus, slot, function, 2);
    vbe_write(bar2, BOCHS_DISPI_EN, 0x00);
    struct display_abstract display;
    display.interface = HAL_DISPLAY_BGA;
    display.specific.bga.bar2 = bar2;
    display.specific.bga.fb = (void *) (uintptr_t) pci_get_bar(bus, slot, function, 0);
    hal_submit(&display);
}

void bochs_display_init() {
    print("BGA: Initializing controllers");
    // First initialize all the VGA compatible Bochs displays
    for (size_t i = 0; i < SIZE_MAX; i++) {
        uint8_t bus;
        uint8_t slot;
        uint8_t function;
        if (pci_get_device(0x03, 0x00, 0x00, &bus, &slot, &function, i) == 0) {
            uint16_t vendor = pci_cfg_read_word(bus, slot, function, PCI_CFG_VENDOR);
            uint16_t device = pci_cfg_read_word(bus, slot, function, PCI_CFG_DEVICE);
            if (vendor == BOCHS_DISPLAY_VENDOR && device == BOCHS_DISPLAY_DEVICE) {
                print("BGA: VGA Compatible controller found at PCI Bus %d Slot %d Function %d", bus, slot, function);
                vga_compat_controller_init(bus, slot, function);
            }
        } else {
            break;
        }
    }
    // Then all the non-compatibles
    for (size_t i = 0; i < SIZE_MAX; i++) {
        uint8_t bus;
        uint8_t slot;
        uint8_t function;
        if (pci_get_device(0x03, 0x80, 0x00, &bus, &slot, &function, i) == 0) {
            uint16_t vendor = pci_cfg_read_word(bus, slot, function, PCI_CFG_VENDOR);
            uint16_t device = pci_cfg_read_word(bus, slot, function, PCI_CFG_DEVICE);
            if (vendor == BOCHS_DISPLAY_VENDOR && device == BOCHS_DISPLAY_DEVICE) {
                print("BGA: Non-VGA Compatible controller found at PCI Bus %d Slot %d Function %d", bus, slot, function);
                non_vga_compat_controller_init(bus, slot, function);
            }
        } else {
            break;
        }
    }
    print("BGA: Finished initializing controllers");
}

/* Functions for VGA Compatible BGA displays */

void bochs_display_vga_regs_write(volatile uint8_t *bar2, struct vga_mode *mode) {
    vbe_write(bar2, BOCHS_DISPI_EN, 0x00);
    vbe_write(bar2, BOCHS_DISPI_EN, 0x00);
    gfx_write(bar2, VGA_GFX_SET_RESET, 0x00);
    gfx_write(bar2, VGA_GFX_EN_SET_RESET, 0x00);
    seq_write(bar2, VGA_SEQ_CLOCKING, seq_read(bar2, VGA_SEQ_CLOCKING) | (1 << 5));
    seq_write(bar2, VGA_SEQ_MAP_MASK, 0x00);
    misc_write(bar2, misc_read(bar2) & ~(1 << 1));
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_AC_ADDR), 0x00);
    dac_write(bar2, 0, mode->pallete, mode->pallete_entries);
    for (int i = 0; i < mode->attr_len; i++) {
        attr_write(bar2, mode->attr[i][0], mode->attr[i][1]);
    }
    // Set the mode
    for (int i = 0; i < mode->seq_len; i++) {
        seq_write(bar2, mode->seq[i][0], mode->seq[i][1]);
    }
    for (int i = 0; i < mode->gfx_len; i++) {
        gfx_write(bar2, mode->gfx[i][0], mode->gfx[i][1]);
    }
    // Unlock CRTC registers
    crtc_write(bar2, VGA_CRTC_VERTICAL_RETRACE_END, crtc_read(bar2, VGA_CRTC_VERTICAL_RETRACE_END) & ~(1 << 7));
    for (int i = 0; i < mode->crtc_len; i++) {
        if (mode->crtc[i][0] == VGA_CRTC_VERTICAL_RETRACE_END) {
            crtc_write(bar2, mode->crtc[i][0], mode->crtc[i][1] & ~(1 << 1));
        } else {
            crtc_write(bar2, mode->crtc[i][0], mode->crtc[i][1]);
        }
    }
    // Lock them again
    crtc_write(bar2, VGA_CRTC_VERTICAL_RETRACE_END, crtc_read(bar2, VGA_CRTC_VERTICAL_RETRACE_END) | (1 << 7));
    misc_write(bar2, mode->misc);
    // Enable display
    reg_write_byte(bar2, VGA_REG_OFFSET(VGA_AC_ADDR), 0x20);
}

void bochs_display_vga_font_write(volatile uint8_t *bar2, const void *font, int height) {
    vga_font_access(bar2, 1);
    for (int i = 0; i < 256; i++) {
        memcpy((void *) (0xa0000 + (32 * i)), (void *) ((uintptr_t) font + (height * i)), height);
    }
    vga_font_access(bar2, 0);
}

void bochs_display_vga_font_read(volatile uint8_t *bar2, void *font, int height) {
    vga_font_access(bar2, 1);
    for (int i = 0; i < 256; i++) {
        memcpy((void *) ((uintptr_t) font + (height * i)), (void *) (0xa0000 + (32 * i)), height);
    }
    vga_font_access(bar2, 0);
}

/* Native BGA display functions */

void bochs_display_high_res(volatile uint8_t *bar2, int width, int height, int bpp, int clear) {
    vbe_write(bar2, BOCHS_DISPI_EN, 0x00);
    vbe_write(bar2, BOCHS_DISPI_XRES, width);
    vbe_write(bar2, BOCHS_DISPI_YRES, height);
    vbe_write(bar2, BOCHS_DISPI_BPP, bpp);
    vbe_write(bar2, BOCHS_DISPI_EN, 0x01 | (!clear ? (1 << 7) : 0x00));
}

/* HAL Functions */

static int hal_resolution(struct display_abstract *this, int width, int height, int bpp, int clear, int text, int vga_mode) {
    int pitch = width * (bpp / 8);
    if (this->interface == HAL_DISPLAY_VGA_BGA && vga_mode) {
        int bufsize;
        if (width == 640 && height == 400 && bpp == 4 && text) {
            bochs_display_vga_regs_write(this->specific.bga.bar2, &vga_mode_80x25x16_text);
            this->common.buffer = (void *) 0xb8000;
            bufsize = 80 * 25 * 2;
        } else if (width == 320 && height == 200 && bpp == 8 && !text) {
            bochs_display_vga_regs_write(this->specific.bga.bar2, &vga_mode_320x200x256_linear);
            this->common.buffer = (void *) 0xa0000;
            bufsize = 320 * 200;
        } else {
            return HAL_DISPLAY_ENORES;
        }
        if (clear) {
            for (int i = 0; i < bufsize; i++) {
                *((uint8_t *) this->common.buffer + i) = 0x00;
            }
        }
        this->properties.vga_mode = 1;
    } else if (!vga_mode) {
        bochs_display_high_res(this->specific.bga.bar2, width, height, bpp, clear);
        this->common.buffer = this->specific.bga.fb;
        this->properties.vga_mode = 0;
    } else {
        return HAL_DISPLAY_ENORES;
    }
    this->common.width = width;
    this->common.height = height;
    this->common.bpp = bpp;
    this->common.pitch = pitch;
    this->properties.text = text;
    return HAL_DISPLAY_ESUCCESS;
}

static int hal_font_get(struct display_abstract *this, const void **font, int *width, int *height) {
    *font = this->font.font;
    *width = this->font.width;
    *height = this->font.height;
    return HAL_DISPLAY_ESUCCESS;
}

static int hal_font_set(struct display_abstract *this, const void *font, int width, int height) {
    this->font.font = font;
    this->font.width = width;
    this->font.height = height;
    if (this->properties.vga_mode) {
        bochs_display_vga_font_write(this->specific.bga.bar2, font, height);
    }
    return HAL_DISPLAY_ESUCCESS;
}

static void hal_submit(struct display_abstract *display) {
    display->ops.resolution = hal_resolution;
    display->ops.font_get = hal_font_get;
    display->ops.font_set = hal_font_set;
    hal_display_submit(display);
}
