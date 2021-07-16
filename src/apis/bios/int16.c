#include <apis/bios/bda.h>
#include <apis/bios/handlers.h>

void apis_bios_int16(struct apis_bios_regs *regs) {
    (void) regs;
    for (;;) {}
}
