#include <cpu/gdt.h>
#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/dram.h>
#include <drivers/fw_cfg.h>
#include <drivers/lpc.h>
#include <drivers/ramfb.h>
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
    if (ramfb_detect() == 0) {
        print("atiebios: ramfb detected! setting a resolution, and drawing something to it :D");
        ramfb_set_resolution(1024, 768, 32);
        uint32_t *framebuffer = (uint32_t *) ramfb_get_framebuffer();
        for (int i = 0; i < 1024 * 768 * 2; i++) {
            framebuffer[i] = 0xabcdef;
        }
    }
    for (;;);
}
