#include <apis/bios/handlers.h>
#include <cpu/pio.h>

void apis_bios_int19(struct apis_bios_regs *regs) {
    (void) regs;
    // Just reset for now, it's enough
    outb(0x64, 0xfe);
}
