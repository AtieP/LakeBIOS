#include <print.h>
#include <drivers/dram.h>
#include <drivers/lpc.h>

__attribute__((__section__(".c_init"), used))
void bios_main() {
    dram_unlock_bios();
    for (;;);
}
