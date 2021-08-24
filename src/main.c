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
#include <hal/disk.h>
#include <hal/display.h>
#include <paravirt/qemu.h>
#include <tools/alloc.h>
#include <tools/bswap.h>
#include <tools/print.h>
#include <tools/string.h>

#include <drivers/pci.h>
#include <drivers/video/romfont.h>
#include <drivers/video/bochs_display.h>
#include <drivers/video/vga_modes.h>
#include <drivers/video/vmware_vga.h>

static void puts_display(const char *string) {
    static int x = 0;
    static int y = 0;
    while (*string) {
        if (*string == '\n') {
            x = 0;
            y++;
            string++;
            continue;
        }
        if (x >= 80) {
            x = 0;
            y++;
        }
        hal_display_plot_char(0x00, *string, x, y, 0x00, 0x07);
        x++;
        string++;
    }
}

__attribute__((__section__(".bios_init"), __used__))
void bios_main() {
    // Force a hard reboot
    if (rtc_reset_status_get() != 0x00) {
        // PCI reset
        uint8_t value = inb(0xcf9) & ~6;
        outb(0xcf9, value | 2);
        outb(0xcf9, value | 6);
        for (;;) {}
    }
    uint16_t vendor_id = pci_cfg_read_word(0, 0, 0, PCI_CFG_VENDOR);
    uint16_t device_id = pci_cfg_read_word(0, 0, 0, PCI_CFG_DEVICE);
    if (vendor_id == I440FX_PMC_VENDOR && device_id == I440FX_PMC_DEVICE) {
        print("i440fx chipset detected, initializing");
        i440fx_init();
    } else if (vendor_id == Q35_DRAM_VENDOR && device_id == Q35_DRAM_DEVICE) {
        print("q35 chipset detected, initializing");
        q35_init();
    } else {
        print("Sorry, unknown chipset with host bridge vendor %x device %x", vendor_id, device_id);
        for (;;) {}
    }
    gdt_craft();
    gdt_reload(GDT_32_CS, GDT_32_DS);
    // Print amount of memory
    print("KiBs of memory between 0M and 1M:  %d", rtc_get_low_mem() / 1024);
    print("KiBs of memory between 1M and 16M: %d", rtc_get_ext1_mem() / 1024);
    print("KiBs of memory between 16M and 4G: %d", rtc_get_ext2_mem() / 1024);
    // Set up allocator
    alloc_setup();
    // AHCI
    ahci_init();
    // NVME
    nvme_init();
    // Bochs displays
    bochs_display_init();
    // VMWare VGA
    vmware_vga_init();
    // hal_display_resolution(0x00, 640, 400, 4, 1, 1, 1);
    hal_display_resolution(0x00, 640, 400, 32, 1, 0, 0);
    hal_display_font_set(0x00, &romfont_8x16, 8, 16);
    puts_display("LakeBIOS - x86 Firmware\nMade by Atie - https://github.com/AtieP/LakeBIOS\n\n");
    // Populate real mode handlers (maybe move this somewhere else)
    uint16_t segment = 0xf000;
    uint16_t offset = 0xd000;
    uint16_t *ivt = (uint16_t *) 0x00;
    for (int i = 0; i < 512; i++) {
        if (i % 2) {
            ivt[i] = segment;
        } else {
            ivt[i] = offset;
            offset += 16;
        }
    }
    rtc_reset_status_set(0x01);
    print("POST finished");
    // Load bootsector from first disk
    puts_display("Booting...\n");
    if (hal_disk_rw(0x80, (void *) 0x7c00, 0, 512, 0) == 0) {
        uint16_t *bootsector = (uint16_t *) 0x7c00;
        if (bootsector[255] == 0xaa55) {
            print("Bootable drive found, jumping to it!\n\n");
            // Jump to it
            // Todo: modify this, this is crusty.
            __asm__ volatile(
                "movb $0xfb, (0x7bff)\n\t"
                "jmp $0x08,$1f\n\t"
                "1:\n\t"
                "mov %%cr0, %%eax\n\t"
                "and $~1, %%eax\n\t"
                "mov %%eax, %%cr0\n\t"
                "xor %%ax, %%ax\n\t"
                "mov %%ax, %%ds\n\t"
                "mov %%ax, %%es\n\t"
                "mov %%ax, %%fs\n\t"
                "mov %%ax, %%gs\n\t"
                "mov %%ax, %%ss\n\t"
                "mov $0x80, %%dl\n\t"
                "mov $0x7b00, %%esp\n\t"
                "jmp $0x00,$0x7bff\n\t"
                ::: "eax"
            );
        }
    }
    puts_display("No bootable drive found. Halting.");
    print("No bootable drive found.");
    for (;;);
}
