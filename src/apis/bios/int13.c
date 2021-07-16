#include <apis/bios/bda.h>
#include <apis/bios/handlers.h>

void apis_bios_int13(struct apis_bios_regs *regs) {
    // Not supported for now...
    if (regs->dl & (1 << 7)) {
        api_bios_bda_disk_status_set(0x01);
    } else {
        api_bios_bda_floppy_status_set(0x01);
    }
    regs->ah = 0x01;
    regs->eflags |= 1;
}
