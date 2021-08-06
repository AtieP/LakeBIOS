#include <drivers/video/vga_io.h>
#include <drivers/video/vga_std.h>
#include <tools/string.h>

static void lock_crtc() {
    vga_crtc_write(VGA_CRTC_VERTICAL_RETRACE_END, vga_crtc_read(VGA_CRTC_VERTICAL_RETRACE_END) | (1 << 7));
}

static void unlock_crtc() {
    vga_crtc_write(VGA_CRTC_VERTICAL_RETRACE_END, vga_crtc_read(VGA_CRTC_VERTICAL_RETRACE_END) & ~(1 << 7));
}

// Flushes the VGA registers
void vga_regs_write(uint8_t misc, const uint8_t (*seq)[2], int seq_len, const uint8_t (*crtc)[2], int crtc_len, const uint8_t (*gfx)[2], int gfx_len, const uint8_t (*attr)[2], int attr_len, const uint8_t (*pallete)[3], int pallete_entries) {
    static int initialized = 0;
    if (initialized) {
        // Disable planes 0-3
        vga_gfx_write(VGA_GFX_SET_RESET, 0x00);
        vga_gfx_write(VGA_GFX_EN_SET_RESET, 0x00);
        // Disable display and turn off planes 0-3
        vga_seq_write(VGA_SEQ_CLOCKING, vga_seq_read(0x01) | (1 << 5));
        vga_seq_write(VGA_SEQ_MAP_MASK, 0x00);
        // Disable RAM decoding
        vga_misc_write(vga_misc_read() & ~(1 << 1));
        outb(VGA_AC_WRITE, 0x00);
    } else {
        initialized = 1;
    }
    vga_misc_write(misc);
    unlock_crtc();
    for (int i = 0; i < seq_len; i++) {
        vga_seq_write(seq[i][0], seq[i][1]);
    }
    for (int i = 0; i < crtc_len; i++) {
        // Avoid accidental locking of CRTC registers
        if (crtc[i][0] == VGA_CRTC_VERTICAL_RETRACE_END) {
            vga_crtc_write(crtc[i][0], crtc[i][1] & ~(1 << 7));
        } else {
            vga_crtc_write(crtc[i][0], crtc[i][1]);
        }
    }
    for (int i = 0; i < gfx_len; i++) {
        vga_gfx_write(gfx[i][0], gfx[i][1]);
    }
    for (int i = 0; i < attr_len; i++) {
        vga_attr_write(attr[i][0], attr[i][1]);
    }
    vga_dac_write(0, pallete, pallete_entries);
    lock_crtc();
    // Enable display
    outb(VGA_AC_WRITE, 0x20);
}

void vga_font_write(const uint8_t *font, int height) {
    uint8_t seq_reset = vga_seq_read(VGA_SEQ_RESET);
    uint8_t seq_map_mask = vga_seq_read(VGA_SEQ_MAP_MASK);
    uint8_t seq_mem_mode = vga_seq_read(VGA_SEQ_MEM_MODE);
    uint8_t gfx_read_map = vga_gfx_read(VGA_GFX_READ_MAP_SEL);
    uint8_t gfx_mode = vga_gfx_read(VGA_GFX_MODE);
    uint8_t gfx_misc = vga_gfx_read(VGA_GFX_MISC);
    vga_seq_write(VGA_SEQ_RESET, 0x01);
    vga_seq_write(VGA_SEQ_MAP_MASK, 0x04);
    vga_seq_write(VGA_SEQ_MEM_MODE, 0x06);
    vga_seq_write(VGA_SEQ_RESET, 0x03);
    vga_gfx_write(VGA_GFX_READ_MAP_SEL, 0x02);
    vga_gfx_write(VGA_GFX_MODE, 0x00);
    vga_gfx_write(VGA_GFX_MISC, 0x04);
    // Copy font into memory
    for (int i = 0; i < 256; i++) {
        memcpy((void *) (0xaa000 + (i * 32)), &font[height * i], height);
    }
    // Restore
    vga_seq_write(VGA_SEQ_RESET, seq_reset);
    vga_seq_write(VGA_SEQ_MAP_MASK, seq_map_mask);
    vga_seq_write(VGA_SEQ_MEM_MODE, seq_mem_mode);
    vga_gfx_write(VGA_GFX_READ_MAP_SEL, gfx_read_map);
    vga_gfx_write(VGA_GFX_MODE, gfx_mode);
    vga_gfx_write(VGA_GFX_MISC, gfx_misc);
}

void vga_cursor_set_pos(uint16_t pos) {
    vga_crtc_write(VGA_CRTC_CURSOR_LOC_LO, (uint8_t) pos);
    vga_crtc_write(VGA_CRTC_CURSOR_LOC_HI, (uint8_t) (pos >> 8));
}

void vga_cursor_get_pos(uint16_t *pos) {
    uint8_t lo = vga_crtc_read(VGA_CRTC_CURSOR_LOC_LO);
    uint8_t hi = vga_crtc_read(VGA_CRTC_CURSOR_LOC_HI);
    *pos = ((uint16_t) hi << 8) | lo;
}

void vga_cursor_set_shape(uint8_t start, uint8_t end, uint8_t flags) {
    vga_crtc_write(VGA_CRTC_CURSOR_START, start | (flags << 5));
    vga_crtc_write(VGA_CRTC_CURSOR_END, end);
}

void vga_cursor_get_shape(uint8_t *start, uint8_t *end, uint8_t *flags) {
    uint8_t start_flags = vga_crtc_read(VGA_CRTC_CURSOR_START);
    *start = start_flags & 0b11111;
    *flags = (start_flags >> 5) & 0b11;
    *end = vga_crtc_read(VGA_CRTC_CURSOR_END);
}
