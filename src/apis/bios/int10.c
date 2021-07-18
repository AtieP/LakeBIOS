#include <cpu/pio.h>
#include <apis/bios/handlers.h>
#include <drivers/video/romfont.h>

// BIG TODO: ACTUALLY USE THE DISPLAY!!!!!! (ramfb doesnt count)

static int x = 0;
static int y = 0;
static uint32_t *fb = (uint32_t *) 0x100000;

static void handle_0e(struct apis_bios_regs *regs) {
    if (regs->al == '\n') {
        goto newline;
    } else if (regs->al == '\r') {
        x = 0;
        return;
    }
    const uint8_t *glyph = &romfont_8x16[regs->al * 16];
    for (int i = 0; i < 16; i++) {
        for (int j = 7; j >= 0; j--) {
            fb[(y * 600) + x] = glyph[i] & (1 << j) ? 0xaaaaaa : 0x000000;
            x++;
        }
        y++;
        x -= 8;
    }
    y -= 16;
    x += 8;
    if (x >= 600) {
        x = 0;
newline:
        y += 16;
    }
}

void apis_bios_int10(struct apis_bios_regs *regs) {
    uint8_t ah = regs->ah;
    if (ah == 0x0e) {
        handle_0e(regs);
    }
}
