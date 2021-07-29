#include <cpu/pio.h>
#include <drivers/video/vga_bochs.h>
#include <drivers/pci.h>

static inline void bga_write(uint16_t index, uint16_t value) {
    outw(BGA_ADDR, index);
    outw(BGA_DATA, value);
}

static inline uint16_t bga_read(uint16_t index) {
    outw(BGA_ADDR, index);
    return inw(BGA_DATA);
}

int bga_detect() {
    uint16_t ver = bga_read(BDA_ID);
    if (ver == 0xb0c0 || ver == 0xb0c1 || ver == 0xb0c2 || ver == 0xb0c3 || ver == 0xb0c4 || ver == 0xb0c5) {
        return 0;
    } else {
        return -1;
    }
}

void *bga_get_fb() {
    uint8_t bus;
    uint8_t slot;
    uint8_t function;
    if (pci_get_device(3, 0, 0, &bus, &slot, &function, 0) != 0) {
        return NULL;
    }
    uint16_t vendor = pci_cfg_read_word(bus, slot, function, PCI_CFG_VENDOR);
    uint16_t device = pci_cfg_read_word(bus, slot, function, PCI_CFG_DEVICE);
    if (vendor == 0x1234 && device == 0x1111) {
        // QEMU
        return (void *) (uint32_t) pci_get_bar(bus, slot, function, 0);
    } else if (vendor == 0x15ad && device == 0x0405) {
        // VMWare
        return (void *) (uint32_t) pci_get_bar(bus, slot, function, 1);
    } else {
        // Unknown, if there are more known vendors add them!
        return NULL;
    }
}

void bga_resolution(int x, int y, int bpp, int clear) {
    bga_write(BGA_ENABLE, 0x00);
    bga_write(BGA_X, x);
    bga_write(BGA_Y, y);
    bga_write(BGA_BPP, bpp);
    bga_write(BGA_ENABLE, 0x01 | (!clear ? 0x80 : 0x00));
}

void bga_deinit() {
    bga_write(BGA_ENABLE, 0x00);
}
