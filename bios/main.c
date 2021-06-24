#include <cpu/gdt.h>
#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/dram.h>
#include <drivers/lpc.h>
#include <tools/print.h>
#include <tools/string.h>

__attribute__((__section__(".c_init"), used))
void bios_main() {
    dram_unlock_bios();
    gdt_craft();
    gdt_reload();
    smm_init();
    for (;;);
}
