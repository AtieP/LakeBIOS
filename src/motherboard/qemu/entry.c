#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/bus/pci.h>
#include <drivers/clock/rtc.h>
#include <drivers/hid/ps2.h>
#include <drivers/irqs/pic.h>
#include <motherboard/qemu/rtc_ext.h>
#include <motherboard/qemu/ich9/acpi.h>
#include <motherboard/qemu/ich9/lpc.h>
#include <motherboard/qemu/q35/dram.h>
#include <hal/power.h>
#include <tools/alloc.h>
#include <tools/print.h>
#include <tools/string.h>
#include <tools/wait.h>

// Devices: only the _init functions in use, remove header if not inited anymore!
#include <drivers/storage/ahci.h>
#include <drivers/storage/nvme.h>
#include <drivers/video/bochs_display.h>
#include <drivers/video/vmware_vga.h>

// Candy, remove later
#include <drivers/video/romfont.h>
#include <hal/display.h>

extern char smm_trampoline_start[];
extern char smm_trampoline_end[];

struct pci_bar_window pci_mem_window = {0};
struct pci_bar_window pci_mem_window_high = {0};
struct pci_bar_window pci_io_window = {0};
struct pci_bar_window pci_pref_window = {0};
struct pci_bar_window pci_pref_window_high = {0};

#ifdef QEMU_I440FX_PIIX
static void qemu_i440fx_piix_init() {
    print("Not implemented yet. Halting");
    for (;;) {}
}
#endif


#ifdef QEMU_Q35_ICH9
static int qemu_q35_ich9_reset(struct power_abstract *power_abstract) {
    (void) power_abstract;
    uint8_t cf9 = inb(0xcf9) & ~0x0e;
    outb(0xcf9, cf9 | 0x02);
    udelay(50);
    outb(0xcf9, cf9 | 0x0e);
    udelay(50);
    return 0;
}

static int qemu_q35_ich9_hal_power_s3(struct power_abstract *power_abstract) {
    (void) power_abstract;
    return qemu_ich9_acpi_pm1a_cnt_slp(3);
}

static int qemu_q35_ich9_hal_power_s4(struct power_abstract *power_abstract) {
    (void) power_abstract;
    return qemu_ich9_acpi_pm1a_cnt_slp(4);
}

static int qemu_q35_ich9_hal_power_s5(struct power_abstract *power_abstract) {
    (void) power_abstract;
    return qemu_ich9_acpi_pm1a_cnt_slp(5);
}

static uint8_t qemu_q35_ich9_get_int_line(int pin, uint8_t bus, uint8_t slot, uint8_t function) {
    (void) bus;
    (void) function;
    if (pin == 0x00 || pin > 0x04) {
        return 0xff;
    }
    if (slot <= 24) {
        return qemu_ich9_lpc_pirq_map[slot & 3];
    }
    return qemu_ich9_lpc_pirq_map[pin - 1];
}

static void qemu_q35_ich9_init() {
    // Memory (this unlocks BIOS data)
    qemu_q35_dram_pam_unlock(5);
    qemu_q35_dram_pam_unlock(6);
    memcpy((void *) 0xe0000, (const void *) (0xfffe0000), 0x10000);
    // SMM
    qemu_q35_dram_smram_en();
    qemu_q35_dram_tseg_set_size(0);
    qemu_q35_dram_smram_open();
    memcpy((void *) (0x30000 + SMM_SMBASE_HANDLER_OFFSET), smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    memcpy((void *) (0xa0000 + SMM_SMBASE_HANDLER_OFFSET), smm_trampoline_start, smm_trampoline_end - smm_trampoline_start);
    qemu_ich9_lpc_pmbase(QEMU_ICH9_ACPI_PMBASE);
    qemu_ich9_lpc_acpi_cntl_pmbase_en();
    qemu_ich9_acpi_smi_en_set(QEMU_ICH9_ACPI_SMI_APMC_EN | QEMU_ICH9_ACPI_SMI_GLB);
    outb(0xb2, 0x01); // relocate SMBASE
    qemu_q35_dram_smram_close();
    qemu_q35_dram_smram_lock();
    // Interrupts
    pic_init(0x08, 0x70);
    for (int i = 0; i <= 8; i++) {
        qemu_ich9_lpc_pirq_route_pic(i);
        qemu_ich9_lpc_pirq_route(i, qemu_ich9_lpc_pirq_map[i]);
    }
    // PCI
    qemu_q35_dram_pciexbar(QEMU_Q35_PCIEXBAR, QEMU_Q35_DRAM_PCIEXBAR_256MB);
    // The MMIO windows have two halves for us: the lower one for Memory, and the higher one for Prefetchable
    uint64_t mem32_base = (uint64_t) 0x1000000 + (qemu_rtc_ext_ext2_mem_kb() * 1024);
    uint64_t mem32_limit = mem32_base + ((QEMU_Q35_PCIEXBAR - mem32_base) / 2);
    uint64_t pref32_base = mem32_limit;
    uint64_t pref32_limit = QEMU_Q35_PCIEXBAR;
    uint64_t mem64_base = 0x100000000 + (qemu_rtc_ext_high_mem_kb() * 1024);
    uint64_t mem64_limit = mem64_base + ((0x1000000000 - mem64_base) / 2);
    uint64_t pref64_base = mem64_limit;
    uint64_t pref64_limit = 0x1000000000; // 64GB
    uint64_t io_base = 0x1000;
    uint64_t io_size = 0xefff;
    pci_mem_window.orig_base = mem32_base;
    pci_mem_window.base = mem32_base;
    pci_mem_window.limit = mem32_limit;
    pci_mem_window.next = &pci_mem_window_high;
    pci_mem_window_high.orig_base = mem64_base;
    pci_mem_window_high.base = mem64_base;
    pci_mem_window_high.limit = mem64_limit;
    pci_mem_window_high.next = NULL;
    pci_pref_window.orig_base = pref32_base;
    pci_pref_window.base = pref32_base;
    pci_pref_window.limit = pref32_limit;
    pci_pref_window.next = &pci_pref_window_high;
    pci_pref_window_high.orig_base = pref64_base;
    pci_pref_window_high.base = pref64_base;
    pci_pref_window_high.limit = pref64_limit;
    pci_pref_window_high.next = NULL;
    pci_io_window.orig_base = io_base;
    pci_io_window.base = io_base;
    pci_io_window.limit = io_base + io_size;
    pci_io_window.next = NULL;
    pci_setup(&pci_mem_window, &pci_io_window, &pci_pref_window, qemu_q35_ich9_get_int_line);
    // ISA devices
    ps2_init();
    // ACPI
    qemu_ich9_lpc_acpi_sci_route(9);
    struct power_abstract power_hal;
    power_hal.interface = HAL_POWER_QEMU_Q35_ICH9;
    power_hal.ops.reset = qemu_q35_ich9_reset;
    power_hal.ops.resume = NULL;
    power_hal.ops.s1 = NULL;
    power_hal.ops.s2 = NULL;
    power_hal.ops.s3 = qemu_q35_ich9_hal_power_s3;
    power_hal.ops.s4 = qemu_q35_ich9_hal_power_s4;
    power_hal.ops.s5 = qemu_q35_ich9_hal_power_s5;
    hal_power_submit(&power_hal);
    // Others
    alloc_setup((qemu_rtc_ext_conv_mem_kb() * 1024) - HEAP_SIZE);
    // PCI devices
    ahci_init();
    nvme_init();
    bochs_display_init();
    vmware_vga_init();
}
#endif

__attribute__((__section__(".bios_init"), __used__))
void qemu_bios_entry() {
    // Ensure this is QEMU
    uint16_t host_bridge_subsystem_vendor = pci_cfg_read_word(0, 0, 0, PCI_CFG_SUBSYSTEM_VENDOR);
    uint16_t host_bridge_subsystem_device = pci_cfg_read_word(0, 0, 0, PCI_CFG_SUBSYSTEM_DEVICE);
    if (host_bridge_subsystem_vendor != 0x1af4 || host_bridge_subsystem_device != 0x1100) {
        print("Not in QEMU. Halting");
        for (;;) {}
    }
    // Initialize chipset
    uint16_t north_bridge_vendor = pci_cfg_read_word(0, 0, 0, PCI_CFG_VENDOR);
    uint16_t north_bridge_device = pci_cfg_read_word(0, 0, 0, PCI_CFG_DEVICE);

#if defined QEMU_I440FX_PIIX && defined QEMU_Q35_ICH9
    if (north_bridge_vendor == 0x8086 && north_bridge_device == 0x1237) {
        print("QEMU I440FX-PIIX machine found, initializing");
        qemu_i440fx_piix_init();
    } else if (north_bridge_vendor == QEMU_Q35_DRAM_VENDOR && north_bridge_device == QEMU_Q35_DRAM_DEVICE) {
        print("QEMU Q35-ICH9 machine found, initializing");
        qemu_q35_ich9_init();
    } else {
        print("Could not detect QEMU machine (Not I440FX-PIIX or Q35-ICH9). Halting");
        for (;;) {}
    }
#endif

#if defined QEMU_I440FX_PIIX && !defined QEMU_Q35_ICH9
    if (north_bridge_vendor == 0x8086 && north_bridge_device == 0x1237) {
        print("QEMU I440FX-PIIX machine found, initializing");
        qemu_i440fx_piix_init();
    } else {
        print("Could not detect QEMU machine (Not I440FX-PIIX). Halting");
        for (;;) {}
    }
#endif

#if !defined QEMU_I440FX_PIIX && defined QEMU_Q35_ICH9
    if (north_bridge_vendor == QEMU_Q35_DRAM_VENDOR && north_bridge_device == QEMU_Q35_DRAM_DEVICE) {
        print("QEMU Q35-ICH9 machine found, initializing");
        qemu_q35_ich9_init();
    } else {
        print("Could not detect QEMU machine (Not Q35-ICH9). Halting");
        for (;;) {}
    }
#endif
    print("POST finished");
    // This is candy. Remove later!
    hal_display_resolution(0x00, 640, 400, 32, 1, 0, 0);
    hal_display_resolution(0x00, 640, 400, 4, 1, 1, 1);
    hal_display_font_set(0x00, &romfont_8x16, 8, 16);
    char message[] = "Hello from LakeBIOS!\nhttps://github.com/AtieP/LakeBIOS\n\nNo boot menu yet, nor OS booting, wait for it!";
    int x = 0;
    int y = 0;
    for (size_t i = 0; i < sizeof(message); i++) {
        char ch = message[i];
        if (ch == '\n') {
            y++;
            x = 0;
            continue;
        }
        if (x == 80) {
            x = 0;
            y++;
        }
        hal_display_plot_char(0x00, ch, x++, y, 0x00, 0x0f);
    }
    for (;;) {}
}