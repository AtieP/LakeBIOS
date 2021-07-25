#include <cpu/pio.h>
#include <apis/bios/handlers.h>
#include <drivers/video/romfont.h>
#include <paravirt/qemu.h>
#include <tools/print.h>

static void handle_00(struct apis_bios_regs *regs) {
    (void) regs;
}

static void handle_02(struct apis_bios_regs *regs) {
    (void) regs;
}

static void handle_03(struct apis_bios_regs *regs) {
    (void) regs;
}

static void handle_0e(struct apis_bios_regs *regs) {
    (void) regs;
}

static void handle_0f(struct apis_bios_regs *regs) {
    (void) regs;
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
