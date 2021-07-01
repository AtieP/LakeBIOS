#include <cpu/gdt.h>
#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/dram.h>
#include <drivers/fw_cfg.h>
#include <drivers/lpc.h>
#include <drivers/pic.h>
#include <drivers/ps2.h>
#include <drivers/ramfb.h>
#include <drivers/rtc.h>
#include <tools/bswap.h>
#include <tools/print.h>
#include <tools/string.h>

#include <drivers/pci.h>

struct {
    uint16_t signature;
    uint32_t file_size;
    uint32_t reserved;
    uint32_t image_offset;
    uint32_t header_size;
    uint32_t image_width;
    uint32_t image_height;
    uint16_t image_planes;
    uint16_t image_bpp;
    uint32_t image_compression;
    uint32_t image_size;
    uint32_t image_xcount;
    uint32_t image_ycount;
} __attribute__((__packed__)) bmp_header;

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
        print("atiebios: ramfb detected!");
        struct fw_cfg_file wallpaper = fw_cfg_get_file("opt/wallpaper");
        if (wallpaper.selector == 0) {
            print("atiebios: no wallpaper found at fw_cfg opt/wallpaper, filling the screen with a color");
            ramfb_set_resolution(1024, 768, 32);
            uint32_t *framebuffer = (uint32_t *) ramfb_get_framebuffer();
            for (int i = 0; i < 1024 * 768 * 2; i++) {
                framebuffer[i] = 0xabcdef;
            }   
        } else {
            fw_cfg_dma_read_selector(wallpaper.selector, &bmp_header, sizeof(bmp_header), 0);
            if (bmp_header.signature != 0x4d42) {
                print("atiebios: wallpaper found, but format is not valid (needs to be a BMP)");
            } else {
                print("atiebios: wallpaper found! width: %d height: %d bpp: %d", bmp_header.image_width, bmp_header.image_height, bmp_header.image_bpp);
                ramfb_set_resolution(bmp_header.image_width, bmp_header.image_height, bmp_header.image_bpp);
                fw_cfg_dma_read_selector(wallpaper.selector, ramfb_get_framebuffer(), bmp_header.image_size, bmp_header.image_offset);
            }
        }
    }
    // PS/2 initialization
    // Disable devices
    ps2_controller_disable_keyb_port();
    ps2_controller_disable_mouse_port();
    // Flush keyboard buffer
    inb(0x60);
    // Disable IRQs and keyboard translation
    ps2_controller_disable_keyb_irqs();
    ps2_controller_disable_mouse_irqs();
    ps2_controller_disable_keyb_translation();
    // Self-test
    if (ps2_controller_self_test() != 0) {
        print("atiebios: could not initialize PS/2 controller because the self test failed");
    } else {
        print("atiebios: PS/2 controller self test passed");
    }
    // Enable devices again
    ps2_controller_enable_keyb_port();
    ps2_controller_enable_mouse_port();
    // Reset them
    if (ps2_keyboard_reset() != 0) {
        print("atiebios: PS/2 keyboard failed to reset");
    }
    if (ps2_mouse_reset() != 0) {
        print("atiebios: PS/2 mouse failed to reset");
    }
    // Print amount of memory
    print("atiebios: KiBs of memory between 0M and 1M:  %d", rtc_get_low_mem() / 1024);
    print("atiebios: KiBs of memory between 1M and 16M: %d", rtc_get_ext1_mem() / 1024);
    print("atiebios: KiBs of memory between 16M and 4G: %d", rtc_get_ext2_mem() / 1024);
    // Enable PIC
    pic_init(8, 0xa0);
    for (;;);
}
