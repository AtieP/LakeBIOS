#include <drivers/fw_cfg.h>
#include <drivers/pci.h>
#include <drivers/ramfb.h>
#include <tools/bswap.h>

static struct {
    uint64_t framebuffer;
    uint32_t model;
    uint32_t flags;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
} __attribute__((__packed__)) ramfb;


int ramfb_detect() {
    if (fw_cfg_get_file("etc/ramfb").selector == 0) {
        return -1;
    } else {
        return 0;
    }
}

void *ramfb_get_framebuffer() {
    return (void *) (0x100000);
}

void ramfb_set_resolution(int width, int height, int bpp) {
    ramfb.width = bswap32(width);
    ramfb.height = bswap32(height);
    ramfb.stride = bswap32(width * (bpp / 8));
    ramfb.framebuffer = bswap64((uint64_t) (uint32_t) ramfb_get_framebuffer());
    ramfb.flags = 0;
    ramfb.model = bswap32(0x34325241);
    // Apply settings
    uint16_t ramfb_selector = fw_cfg_get_file("etc/ramfb").selector;
    fw_cfg_dma_write_selector(ramfb_selector, &ramfb, sizeof(ramfb), 0);
}
