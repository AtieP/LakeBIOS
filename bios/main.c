#include <cpu/gdt.h>
#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/dram.h>
#include <drivers/fw_cfg.h>
#include <drivers/lpc.h>
#include <tools/bswap.h>
#include <tools/print.h>
#include <tools/string.h>

#include <drivers/pci.h>

__attribute__((__section__(".c_init"), used))
void bios_main() {
    dram_unlock_bios();
    gdt_craft();
    gdt_reload();
    smm_init();
    pci_enumerate();
    dram_set_tolud(0xb0000000);
    dram_set_pciexbar(0xb0000000);
    dram_enable_pciexbar();
    print("atiebios: execution ended.");
    for (;;);
}
