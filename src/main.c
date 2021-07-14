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
    gdt_reload(GDT_32_CS, GDT_32_DS);
    if (ps2_init() != 0) {
        print("lakebios: ps2 not initialized successfully. Halting.");
        for (;;) {}
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
    print("lakebios: POST finished");
    // Load bootsector from first disk
    if (hal_disk_rw(0x80, (void *) 0x7c00, 0, 512, 0) == 0) {
        uint16_t *bootsector = (uint16_t *) 0x7c00;
        if (bootsector[255] == 0xaa55) {
            print("lakebios: bootable drive found, jumping to it!\n\n");
            // Jump to it
            // Todo: modify this, this is crusty.
            asm volatile(
                "mov $0x10, %%ax\n\t"
                "mov %%ax, %%ds\n\t"
                "mov %%ax, %%es\n\t"
                "mov %%ax, %%fs\n\t"
                "mov %%ax, %%gs\n\t"
                "mov %%ax, %%ss\n\t"
                "jmp $0x08,$1f\n\t"
                "1:\n\t"
                "mov %%cr0, %%eax\n\t"
                "and $~1, %%eax\n\t"
                "mov %%eax, %%cr0\n\t"
                "jmp $0x00,$0x7c00\n\t"
                ::: "eax"
            );
        }
    }
    print("lakebios: no bootable disk found.");
    for (;;);
}
