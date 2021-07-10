#ifndef __CPU_IDT_H__
#define __CPU_IDT_H__

#include <stdint.h>

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_hi;
} __attribute__((__packed__));

void idt_init();

#endif
