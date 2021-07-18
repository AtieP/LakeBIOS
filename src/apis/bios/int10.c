#include <cpu/pio.h>
#include <apis/bios/handlers.h>
#include <drivers/video/romfont.h>
#include <paravirt/qemu.h>
#include <tools/print.h>

// Emulated for ramfb.
// TODO: BDA

static int width = 600;
static int height = 480;
static int x = 0;
static int y = 0;
static int font_width = 8;
static int font_height = 16;
static uint32_t *fb = (uint32_t *) 0x100000;
static const uint8_t *font = (const uint8_t *) &romfont_8x16;
static int mode = 0x03;

static void handle_00(struct apis_bios_regs *regs) {
    if (regs->al == 0x00 || regs->al == 0x01 || regs->al == 0x04 || regs->al == 0x05) {
        width = 320;
        height = 200;
        font = (const uint8_t *) &romfont_8x8;
    } else if (regs->al == 0x03) {
        width = 600;
        height = 480;
        font = (const uint8_t *) &romfont_8x16;
    }
    mode = regs->al;
    x = 0;
    y = 0;
    qemu_ramfb_resolution(0x100000, width, height, 32);
    for (int i = 0; i < width * height; i++) {
        fb[i] = 0;
    }
}

static void handle_02(struct apis_bios_regs *regs) {
    x = regs->dl * font_width;
    y = regs->dh * font_height;
}

static void handle_03(struct apis_bios_regs *regs) {
    regs->ax = 0;
    regs->ch = 0;
    regs->cl = 0;
    regs->dh = y / font_height;
    regs->dl = x / font_width;
}

static void handle_0e(struct apis_bios_regs *regs) {
    if (regs->al == '\n') {
        goto newline;
    } else if (regs->al == '\r') {
        x = 0;
        return;
    }
    const uint8_t *glyph = &font[regs->al * font_height];
    for (int i = 0; i < font_height; i++) {
        for (int j = font_width - 1; j >= 0; j--) {
            fb[(y * width) + x] = glyph[i] & (1 << j) ? 0xaaaaaa : 0x000000;
            x++;
        }
        y++;
        x -= font_width;
    }
    y -= font_height;
    x += font_width;
    if (x >= width) {
        x = 0;
newline:
        y += font_height;
    }
    if (y >= height - font_height) {
        x = 0;
        y -= font_height;
        int row_size = (width * 4) * font_height / 4;
        int screen_size = (width * 4) * font_height * (height / font_height) / 4;
        for (int i = 0; i < screen_size - row_size; i++) {
            fb[i] = fb[i + row_size];
        }
    }
}

static void handle_0f(struct apis_bios_regs *regs) {
    regs->ah = width / font_width;
    regs->al = mode;
    regs->bh = 0;
}

void apis_bios_int10(struct apis_bios_regs *regs) {
    uint8_t ah = regs->ah;
    if (ah == 0x00) {
        handle_00(regs);
    } else if (ah == 0x02) {
        handle_02(regs);
    } else if (ah == 0x03) {
        handle_03(regs);
    } else if (ah == 0x0e) {
        handle_0e(regs);
    } else if (ah == 0x0f) {
        handle_0f(regs);
    } else {
        print("lakebios: bios: handler not available.");
        for (;;) {}
    }
}
