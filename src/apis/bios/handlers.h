#ifndef __APIS_BIOS_HANDLERS_H__
#define __APIS_BIOS_HANDLERS_H__

#include <stdint.h>

struct apis_bios_regs {
    union {
        struct {
            uint8_t al;
            uint8_t ah;
        };
        struct {
            uint16_t ax;
        };
        uint32_t eax;
    };
    union {
        struct {
            uint8_t bl;
            uint8_t bh;
        };
        struct {
            uint16_t bx;
        };
        uint32_t ebx;
    };
    union {
        struct {
            uint8_t cl;
            uint8_t ch;
        };
        struct {
            uint16_t cx;
        };
        uint32_t ecx;
    };
    union {
        struct {
            uint8_t dl;
            uint8_t dh;
        };
        struct {
            uint16_t dx;
        };
        uint32_t edx;
    };
    union {
        struct {
            uint16_t si;
        };
        uint32_t esi;
    };
    uint32_t edi;
    uint32_t ebp;
    uint32_t eflags;
    uint32_t es;
} __attribute__((__packed__));

void apis_bios_int10(struct apis_bios_regs *regs);
void apis_bios_int13(struct apis_bios_regs *regs);
void apis_bios_int16(struct apis_bios_regs *regs);
void apis_bios_int19(struct apis_bios_regs *regs);

#endif
