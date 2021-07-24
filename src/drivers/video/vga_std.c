#include <drivers/video/vga_io.h>
#include <drivers/video/vga_std.h>

static void lock_crtc() {
    vga_crtc_write(VGA_CRTC_VERTICAL_RETRACE_END, vga_crtc_read(VGA_CRTC_VERTICAL_RETRACE_END) | (1 << 7));
}

static void unlock_crtc() {
    vga_crtc_write(VGA_CRTC_VERTICAL_RETRACE_END, vga_crtc_read(VGA_CRTC_VERTICAL_RETRACE_END) & ~(1 << 7));
}

// Flushes the VGA registers
void vga_regs_write(uint8_t misc, const uint8_t (*seq)[2], int seq_len, const uint8_t (*crtc)[2], int crtc_len, const uint8_t (*gfx)[2], int gfx_len, const uint8_t (*attr)[2], int attr_len) {
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
    lock_crtc();
    // Enable display
    outb(VGA_AC_WRITE, 0x20);
}
