#include <cpu/gdt.h>

struct {
    struct gdt_entry entries[5];
} __attribute__((__packed__)) gdt;

void gdt_craft() {
    gdt.entries[0].limit_low = 0;
    gdt.entries[0].base_low = 0;
    gdt.entries[0].base_mid = 0;
    gdt.entries[0].access = 0;
    gdt.entries[0].granularity = 0;
    gdt.entries[0].base_high = 0;

    gdt.entries[1].limit_low = 0xffff;
    gdt.entries[1].base_low = 0;
    gdt.entries[1].base_mid = 0;
    gdt.entries[1].access = 0x9a;
    gdt.entries[1].granularity = 0x0f;
    gdt.entries[1].base_high = 0;

    gdt.entries[2].limit_low = 0xffff;
    gdt.entries[2].base_low = 0;
    gdt.entries[2].base_mid = 0;
    gdt.entries[2].access = 0x92;
    gdt.entries[2].granularity = 0x0f;
    gdt.entries[2].base_high = 0;

    gdt.entries[3].limit_low = 0xffff;
    gdt.entries[3].base_low = 0;
    gdt.entries[3].base_mid = 0;
    gdt.entries[3].access = 0x9a;
    gdt.entries[3].granularity = 0xcf;
    gdt.entries[3].base_high = 0;

    gdt.entries[4].limit_low = 0xffff;
    gdt.entries[4].base_low = 0;
    gdt.entries[4].base_mid = 0;
    gdt.entries[4].access = 0x92;
    gdt.entries[4].granularity = 0xcf;
    gdt.entries[4].base_high = 0;
}

void gdt_reload(uint32_t cs, uint32_t ds) {
    struct gdt_register gdt_reg;
    gdt_reg.base = (uint32_t) &gdt;
    gdt_reg.limit = sizeof(gdt) - 1;
    __asm__ volatile(
        "lgdt %0\n\t"
        "push %1\n\t"
        "push $1f\n\t"
        "lret\n\t"
        "1:\n\t"
        "mov %2, %%ds\n\t"
        "mov %2, %%es\n\t"
        "mov %2, %%gs\n\t"
        "mov %2, %%fs\n\t"
        "mov %2, %%ss\n\t"
        :: "m"(gdt_reg), "rmi"((uint32_t) cs), "rm"((uint32_t) ds)
        : "memory"
    );
}
