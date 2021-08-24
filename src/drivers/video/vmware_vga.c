#include <stdint.h>
#include <cpu/pio.h>
#include <drivers/video/vmware_vga.h>
#include <drivers/pci.h>
#include <hal/display.h>
#include <tools/print.h>

// TODO:
// 1. Fix the FIFO code
// 2. Sometimes the text misteriously disappears but when you toggle full screen it appears
// 3. Legacy VGA functions

/* Read/Write from/to BAR 0 */

static uint32_t reg_read(uint16_t bar0, size_t index) {
    outd(bar0 + VMWARE_VGA_INDEX, index);
    return ind(bar0 + VMWARE_VGA_VALUE);
}

static void reg_write(uint16_t bar0, size_t index, uint32_t value) {
    outd(bar0 + VMWARE_VGA_INDEX, index);
    outd(bar0 + VMWARE_VGA_VALUE, value);
}

/* Functions for figuring out capabilities */

static int is_capable(uint16_t bar0, uint32_t capability) {
    // Read magic and ensure ID is greater than 0
    if (reg_read(bar0, VMWARE_VGA_ID) < 0x90000001) {
        return 0;
    }
    return 1 ? reg_read(bar0, VMWARE_VGA_CAPABILITIES) & capability : 0;
}

static int is_reg_supported(uint16_t bar0, uint32_t reg) {
    uint32_t magic = reg_read(bar0, VMWARE_VGA_ID);
    if (magic >= 0x90000000 && reg <= 16) {
        return 1;
    } else if (magic >= 0x90000001 && (reg >= 17 && reg <= 32)) {
        return 1;
    } else {
        return 0;
    }
}

/* Initialization functions */

static void hal_submit(struct display_abstract *display);

static void controller_init(uint8_t bus, uint8_t slot, uint8_t function) {
    uint16_t bar0 = (uint16_t) pci_get_bar(bus, slot, function, 0);
    uint32_t *fifo = (uint32_t *) (uintptr_t) pci_get_bar(bus, slot, function, 2);
    reg_write(bar0, VMWARE_VGA_ENABLE, 0);
    // Set up the FIFO buffer
    if (is_reg_supported(bar0, VMWARE_VGA_FIFO_START)) { // Lowest common denominator
        fifo[VMWARE_VGA_FIFO_MIN_IDX] = 4;
        fifo[VMWARE_VGA_FIFO_MAX_IDX] = 16;
        fifo[VMWARE_VGA_FIFO_NEXT_IDX] = 4;
        fifo[VMWARE_VGA_FIFO_STOP_IDX] = 4;
        reg_write(bar0, VMWARE_VGA_CFG_DONE, 1);
    }
    struct display_abstract vmware_vga;
    vmware_vga.interface = HAL_DISPLAY_VMWARE_VGA;
    void *fb = (void *) (uintptr_t) pci_get_bar(bus, slot, function, 1);
    vmware_vga.common.buffer = fb;
    vmware_vga.specific.vmware_vga.bar0 = bar0;
    vmware_vga.specific.vmware_vga.fb = fb;
    vmware_vga.specific.vmware_vga.fifo = fifo;
    hal_submit(&vmware_vga);
}

void vmware_vga_init() {
    // There can be only 1 VMWare VGA device, and it is always VGA compatible
    print("VMWare VGA: Initializing controller");
    for (size_t i = 0; i < SIZE_MAX; i++) {
        uint8_t bus;
        uint8_t slot;
        uint8_t function;
        if (pci_get_device(0x03, 0x00, 0x00, &bus, &slot, &function, i) == 0) {
            uint16_t vendor = pci_cfg_read_word(bus, slot, function, PCI_CFG_VENDOR);
            uint16_t device = pci_cfg_read_word(bus, slot, function, PCI_CFG_DEVICE);
            if (vendor == VMWARE_VGA_VENDOR && device == VMWARE_VGA_DEVICE) {
                print("VMWare VGA: Controller found at PCI Bus %d Slot %d Function %d", bus, slot, function);
                controller_init(bus, slot, function);
            }
        } else {
            break;
        }
    }
    print("VMWare VGA: Finished initializing controller");
}

/* Exposes functions */

void vmware_vga_high_res(uint16_t bar0, int width, int height, int bpp, int *pitch) {
    reg_write(bar0, VMWARE_VGA_ENABLE, 1);
    reg_write(bar0, VMWARE_VGA_WIDTH, width);
    reg_write(bar0, VMWARE_VGA_HEIGHT, height);
    reg_write(bar0, VMWARE_VGA_BPP, bpp);
    if (is_capable(bar0, VMWARE_VGA_CAP_PITCHLOCK)) {
        *pitch = reg_read(bar0, VMWARE_VGA_PITCHLOCK);
    } else {
        *pitch = width * (bpp / 8);
    }
}

/* HAL Functions */

static int hal_resolution(struct display_abstract *this, int width, int height, int bpp, int clear, int text, int vga_mode) {
    if (vga_mode || text) {
        return HAL_DISPLAY_ENOIMPL;
    }
    int pitch;
    vmware_vga_high_res(this->specific.vmware_vga.bar0, width, height, bpp, &pitch);
    this->common.buffer = this->specific.vmware_vga.fb;
    if (clear) {
        for (size_t i = 0; i < (size_t) pitch * height; i++) {
            *((uint8_t *) this->common.buffer) = 0x00;
        }
    }
    this->common.width = width;
    this->common.height = height;
    this->common.bpp = bpp;
    this->common.pitch = pitch;
    this->properties.text = text;
    return HAL_DISPLAY_ESUCCESS;
}

static int hal_font_get(struct display_abstract *this, const void **font, int *width, int *height) {
    *font = this->font.font;
    *width = this->font.width;
    *height = this->font.height;
    return HAL_DISPLAY_ESUCCESS;
}

static int hal_font_set(struct display_abstract *this, const void *font, int width, int height) {
    this->font.font = font;
    this->font.width = width;
    this->font.height = height;
    return HAL_DISPLAY_ESUCCESS;
}

static void hal_submit(struct display_abstract *display) {
    display->ops.resolution = hal_resolution;
    display->ops.font_get = hal_font_get;
    display->ops.font_set = hal_font_set;
    hal_display_submit(display);
}
