#ifndef __DRIVERS_VIDEO_VMWARE_VGA_H__
#define __DRIVERS_VIDEO_VMWARE_VGA_H__

#define VMWARE_VGA_VENDOR 0x15ad
#define VMWARE_VGA_DEVICE 0x0405

#define VMWARE_VGA_INDEX 0x00
#define VMWARE_VGA_VALUE 0x01
#define VMWARE_VGA_BIOS  0x02

// ID 0x00
#define VMWARE_VGA_ID           0x00
#define VMWARE_VGA_ENABLE       0x01
#define VMWARE_VGA_WIDTH        0x02
#define VMWARE_VGA_HEIGHT       0x03
#define VMWARE_VGA_MAX_WIDTH    0x04
#define VMWARE_VGA_MAX_HEIGHT   0x05
#define VMWARE_VGA_BPP          0x06

// ID 0x01 and 0x02
#define VMWARE_VGA_CAPABILITIES 0x11
#define VMWARE_VGA_FIFO_START   0x12
#define VMWARE_VGA_FIFO_SIZE    0x13
#define VMWARE_VGA_CFG_DONE     0x14
#define VMWARE_VGA_CURSOR_X     0x19
#define VMWARE_VGA_CURSOR_Y     0x1a
#define VMWARE_VGA_CURSOR_ON    0x1b
#define VMWARE_VGA_PITCHLOCK    0x20

#define VMWARE_VGA_CAP_PITCHLOCK (1 << 17)

// FIFO
#define VMWARE_VGA_FIFO_MIN_IDX  0x00
#define VMWARE_VGA_FIFO_MAX_IDX  0x01
#define VMWARE_VGA_FIFO_NEXT_IDX 0x02
#define VMWARE_VGA_FIFO_STOP_IDX 0x03

void vmware_vga_init();
void vmware_vga_high_res(uint16_t bar0, int width, int height, int bpp, int *pitch);

#endif
