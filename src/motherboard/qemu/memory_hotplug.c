#include <cpu/pio.h>
#include <motherboard/qemu/memory_hotplug.h>

/*
    Documentation (please update if a newer revision is implemented here):
    https://github.com/qemu/qemu/blob/444fa22549434331db70718f073432ed2057ada8/docs/specs/acpi_mem_hotplug.txt

    (Note: the self detection methods are wack)
*/

static uint16_t memory_hotplug_base = 0;

static uint8_t memory_hotplug_read_byte(uint8_t offset) {
    return inb(memory_hotplug_base + offset);
}

static uint32_t memory_hotplug_read_dword(uint8_t offset) {
    return ind(memory_hotplug_base + offset);
}

static void memory_hotplug_write_byte(uint8_t offset, uint8_t value) {
    outb(memory_hotplug_base + offset, value);
}

static void memory_hotplug_write_dword(uint8_t offset, uint32_t value) {
    outd(memory_hotplug_base + offset, value);
}

void qemu_memory_hotplug_set_io_base(uint16_t base) {
    memory_hotplug_base = base;
}

int qemu_memory_hotplug_exists() {
    // QEMU returns ~0 if port is not backed, so just test
    memory_hotplug_write_dword(QEMU_MEMORY_HOTPLUG_REG_WRITE_SELECT, 0x00);
    uint64_t base = ((uint64_t) memory_hotplug_read_dword(QEMU_MEMORY_HOTPLUG_REG_READ_ADDR_HI) << 32) 
                    | memory_hotplug_read_dword(QEMU_MEMORY_HOTPLUG_REG_READ_ADDR_LO);
    uint64_t length = ((uint64_t) memory_hotplug_read_dword(QEMU_MEMORY_HOTPLUG_REG_READ_SIZE_HI) << 32)
                    | memory_hotplug_read_dword(QEMU_MEMORY_HOTPLUG_REG_READ_SIZE_LO);
    if (~base == 0 && ~length == 0) {
        return 0;
    }
    return 1;
}

uint32_t qemu_memory_hotplug_max_dimm() {
    // QEMU returns 0 if the slot is not valid, and having a 0 length is impossible anyways
    uint32_t i = 0;
    while (1) {
        memory_hotplug_write_dword(QEMU_MEMORY_HOTPLUG_REG_WRITE_SELECT, i);
        uint64_t length = ((uint64_t) memory_hotplug_read_dword(QEMU_MEMORY_HOTPLUG_REG_READ_SIZE_HI) << 32)
                        | memory_hotplug_read_dword(QEMU_MEMORY_HOTPLUG_REG_READ_SIZE_LO);
        if (!length) {
            break;
        } 
        if (i == 0xffffffff) {
            break; // Do not overflow
        }
        i++;
    }
    return i;
}

int qemu_memory_hotplug_dimm_get_info(uint32_t dimm, struct qemu_dimm_info *dimm_info) {
    memory_hotplug_write_dword(QEMU_MEMORY_HOTPLUG_REG_WRITE_SELECT, dimm);
    dimm_info->base = ((uint64_t) memory_hotplug_read_dword(QEMU_MEMORY_HOTPLUG_REG_READ_ADDR_HI) << 32)
                    | memory_hotplug_read_dword(QEMU_MEMORY_HOTPLUG_REG_READ_ADDR_LO);
    dimm_info->length = ((uint64_t) memory_hotplug_read_dword(QEMU_MEMORY_HOTPLUG_REG_READ_SIZE_HI) << 32)
                        | memory_hotplug_read_dword(QEMU_MEMORY_HOTPLUG_REG_READ_SIZE_LO);
    dimm_info->proximity = memory_hotplug_read_dword(QEMU_MEMORY_HOTPLUG_REG_READ_PROXIMITY);
    dimm_info->status = memory_hotplug_read_byte(QEMU_MEMORY_HOTPLUG_REG_READ_STATUS);
    return 0;
}

int qemu_memory_hotplug_dimm_eject(uint32_t dimm) {
    memory_hotplug_write_dword(QEMU_MEMORY_HOTPLUG_REG_WRITE_SELECT, dimm);
    memory_hotplug_write_dword(QEMU_MEMORY_HOTPLUG_REG_WRITE_CONTROL, QEMU_MEMORY_HOTPLUG_CONTROL_EJECT);
    return 0;
}
