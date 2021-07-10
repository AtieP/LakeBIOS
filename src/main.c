#include <hal.h>
#include <chipsets/i440fx.h>
#include <chipsets/q35.h>
#include <cpu/gdt.h>
#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/ahci.h>
#include <drivers/fw_cfg.h>
#include <drivers/nvme.h>
#include <drivers/pic.h>
#include <drivers/ps2.h>
#include <drivers/ramfb.h>
#include <drivers/rtc.h>
#include <tools/alloc.h>
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

__attribute__((__section__(".bios_init"), __used__))
void bios_main() {
    // Chipset specific setup first, first detect what chipset running on
    uint16_t chipset_vendor = pci_cfg_read_word(0, 0, 0, PCI_CFG_VENDOR);
    uint16_t chipset_device = pci_cfg_read_word(0, 0, 0, PCI_CFG_DEVICE);
    if (chipset_vendor == Q35_DRAM_VENDOR && chipset_device == Q35_DRAM_DEVICE) {
        q35_init();
    } else if (chipset_vendor == I440FX_PMC_VENDOR && chipset_device == I440FX_PMC_DEVICE) {
        i440fx_init();
    } else {
        print("Sorry, chipset with host bridge vendor %x device %x not supported", chipset_vendor, chipset_device);
        for (;;);
    }
    gdt_craft();
    gdt_reload();
    if (ramfb_detect() == 0) {
        print("lakebios: ramfb detected!");
        struct fw_cfg_file wallpaper = fw_cfg_get_file("opt/wallpaper");
        if (wallpaper.selector == 0) {
            print("lakebios: no wallpaper found at fw_cfg opt/wallpaper, filling the screen with a color");
            ramfb_set_resolution(1024, 768, 32);
            uint32_t *framebuffer = (uint32_t *) ramfb_get_framebuffer();
            for (int i = 0; i < 1024 * 768 * 2; i++) {
                framebuffer[i] = 0xabcdef;
            }   
        } else {
            fw_cfg_dma_read_selector(wallpaper.selector, &bmp_header, sizeof(bmp_header), 0);
            if (bmp_header.signature != 0x4d42) {
                print("lakebios: wallpaper found, but format is not valid (needs to be a BMP)");
            } else {
                print("lakebios: wallpaper found! width: %d height: %d bpp: %d", bmp_header.image_width, bmp_header.image_height, bmp_header.image_bpp);
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
        print("lakebios: could not initialize PS/2 controller because the self test failed");
    } else {
        print("lakebios: PS/2 controller self test passed");
    }
    // Enable devices again
    ps2_controller_enable_keyb_port();
    ps2_controller_enable_mouse_port();
    // Reset them
    if (ps2_keyboard_reset() != 0) {
        print("lakebios: PS/2 keyboard failed to reset");
    }
    if (ps2_mouse_reset() != 0) {
        print("lakebios: PS/2 mouse failed to reset");
    }
    // Print amount of memory
    print("lakebios: KiBs of memory between 0M and 1M:  %d", rtc_get_low_mem() / 1024);
    print("lakebios: KiBs of memory between 1M and 16M: %d", rtc_get_ext1_mem() / 1024);
    print("lakebios: KiBs of memory between 16M and 4G: %d", rtc_get_ext2_mem() / 1024);
    // Set up allocator
    alloc_setup();
    // Enable PIC
    pic_init(8, 0xa0);
    // AHCI
    ahci_init();
    // NVME
    nvme_init();
    print("lakebios: POST finished");
    for (;;);
}
