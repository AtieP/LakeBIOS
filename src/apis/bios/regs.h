#ifndef __APIS_BIOS_REGS_H__
#define __APIS_BIOS_REGS_H__

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
    union {
        struct {
            uint16_t di;
        };
        uint32_t edi;
    };
    uint32_t ebp;
    uint32_t eflags;
    uint32_t es;
    uint32_t ds;
} __attribute__((__packed__));

#endif
