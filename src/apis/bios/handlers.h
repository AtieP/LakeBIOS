#ifndef __APIS_BIOS_HANDLERS_H__
#define __APIS_BIOS_HANDLERS_H__

#include <stdint.h>

struct apis_bios_regs {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t eflags;
    uint32_t eip;
    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t ss;
} __attribute__((__packed__));

void apis_bios_int10(struct apis_bios_regs *regs);

#endif
