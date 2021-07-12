#include <hal.h>
#include <chipsets/i440fx.h>
#include <chipsets/q35.h>
#include <cpu/gdt.h>
#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/ahci.h>
#include <drivers/nvme.h>
#include <drivers/pic.h>
#include <drivers/ps2.h>
#include <drivers/rtc.h>
#include <paravirt/qemu.h>
#include <tools/alloc.h>
#include <tools/bswap.h>
#include <tools/print.h>
#include <tools/string.h>

#include <drivers/pci.h>

__attribute__((__section__(".bios_init"), __used__))
void bios_main() {
    uint16_t vendor_id = pci_cfg_read_word(0, 0, 0, PCI_CFG_VENDOR);
    uint16_t device_id = pci_cfg_read_word(0, 0, 0, PCI_CFG_DEVICE);
    if (vendor_id == I440FX_PMC_VENDOR && device_id == I440FX_PMC_DEVICE) {
        print("lakebios: i440fx chipset detected, initializing");
        i440fx_init();
    } else if (vendor_id == Q35_DRAM_VENDOR && device_id == Q35_DRAM_DEVICE) {
        print("lakebios: q35 chipset detected, initializing");
        q35_init();
    } else {
        print("lakebios: sorry, unknown chipset with host bridge vendor %x device %x", vendor_id, device_id);
        for (;;) {}
    }
    gdt_craft();
    gdt_reload();
    if (ps2_init() != 0) {
        print("lakebios: ps2 not initialized successfully. Halting.");
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
    // Make something fancy :)
    struct qemu_fw_cfg_file file;
    if (qemu_fw_cfg_get_file("etc/ramfb", &file) == 0) {
        uint16_t ramfb_selector = file.selector;
        struct {
            uint64_t framebuffer;
            uint32_t model;
            uint32_t flags;
            uint32_t width;
            uint32_t height;
            uint32_t stride;
        } __attribute__((__packed__)) ramfb;
        ramfb.width = bswap32(1024);
        ramfb.height = bswap32(768);
        ramfb.stride = bswap32(1024 * (32 / 8));
        ramfb.framebuffer = bswap64((uint64_t) 0x100000);
        ramfb.flags = 0;
        ramfb.model = bswap32(0x34325241);
        qemu_fw_cfg_write(ramfb_selector, &ramfb, sizeof(ramfb), 0);
        uint32_t *fb = (uint32_t *) 0x100000;
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
        if (qemu_fw_cfg_get_file("opt/wallpaper", &file) == 0) {
            qemu_fw_cfg_read(file.selector, &bmp_header, sizeof(bmp_header), 0);
            if (bmp_header.signature == 0x4d42) {
                ramfb.width = bswap32(bmp_header.image_width);
                ramfb.height = bswap32(bmp_header.image_height);
                ramfb.stride = bswap32(bmp_header.image_width * (bmp_header.image_bpp / 8));
                qemu_fw_cfg_write(ramfb_selector, &ramfb, sizeof(ramfb), 0);
                qemu_fw_cfg_read(file.selector, fb, bmp_header.image_size, bmp_header.image_offset);
            }
        } else {
            for (int i = 0; i < 1024 * 768; i++) {
                fb[i] = 0xd3d3d3;
            }
        }
    }
    print("lakebios: POST finished");
    for (;;);
}
