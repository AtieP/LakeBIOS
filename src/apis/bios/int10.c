#include <cpu/pio.h>
#include <apis/bios/handlers.h>

// BIG TODO: ACTUALLY USE THE DISPLAY!!!!!!

static void handle_0e(struct apis_bios_regs *regs) {
    outb(0xe9, (uint8_t) regs->eax);
}

void apis_bios_int10(struct apis_bios_regs *regs) {
    uint8_t ah = (uint8_t) (regs->eax >> 8);
    if (ah == 0x0e) {
        handle_0e(regs);
    }
}
