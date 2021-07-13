#ifndef __CPU_GDT_H__
#define __CPU_GDT_H__

#include <stdint.h>

#define GDT_16_CS 0x08
#define GDT_16_DS 0x10
#define GDT_32_CS 0x18
#define GDT_32_DS 0x20

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((__packed__));

struct gdt_register {
    uint16_t limit;
    uint32_t base;
} __attribute__((__packed__));

void gdt_craft();
void gdt_reload(uint32_t cs, uint32_t ds);

#endif
