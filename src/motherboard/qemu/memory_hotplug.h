#ifndef __MOTHERBOARD_QEMU_MEMORY_HOTPLUG_H__
#define __MOTHERBOARD_QEMU_MEMORY_HOTPLUG_H__

#include <stdint.h>

#define QEMU_MEMORY_HOTPLUG_STATUS_ENABLED  (1 << 0)
#define QEMU_MEMORY_HOTPLUG_STATUS_INSERTED (1 << 1)
#define QEMU_MEMORY_HOTPLUG_STATUS_REMOVED  (1 << 2)

#define QEMU_MEMORY_HOTPLUG_REG_READ_ADDR_LO   0x00
#define QEMU_MEMORY_HOTPLUG_REG_READ_ADDR_HI   0x04
#define QEMU_MEMORY_HOTPLUG_REG_READ_SIZE_LO   0x08
#define QEMU_MEMORY_HOTPLUG_REG_READ_SIZE_HI   0x0c
#define QEMU_MEMORY_HOTPLUG_REG_READ_PROXIMITY 0x10
#define QEMU_MEMORY_HOTPLUG_REG_READ_STATUS    0x14

#define QEMU_MEMORY_HOTPLUG_CONTROL_INSERTED_C (1 << 1)
#define QEMU_MEMORY_HOTPLUG_CONTROL_REMOVED_C  (1 << 2)
#define QEMU_MEMORY_HOTPLUG_CONTROL_EJECT      (1 << 3)

#define QEMU_MEMORY_HOTPLUG_REG_WRITE_SELECT     0x00
#define QEMU_MEMORY_HOTPLUG_REG_WRITE_OST_EVENT  0x04
#define QEMU_MEMORY_HOTPLUG_REG_WRITE_OST_STATUS 0x08
#define QEMU_MEMORY_HOTPLUG_REG_WRITE_CONTROL    0x14

struct qemu_dimm_info {
    uint64_t base;
    uint64_t length;
    uint32_t proximity;
    uint8_t status;
};

void qemu_memory_hotplug_set_io_base(uint16_t base);
int qemu_memory_hotplug_exists();
uint32_t qemu_memory_hotplug_max_dimm();
int qemu_memory_hotplug_dimm_get_info(uint32_t dimm, struct qemu_dimm_info *dimm_info);
int qemu_memory_hotplug_dimm_eject(uint32_t dimm);

#endif
