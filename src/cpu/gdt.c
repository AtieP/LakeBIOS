#include <cpu/gdt.h>

struct {
    struct gdt_entry entries[3];
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
    gdt.entries[1].access = 0b10011010;
    gdt.entries[1].granularity = 0b11001111;
    gdt.entries[1].base_high = 0;

    gdt.entries[2].limit_low = 0xffff;
    gdt.entries[2].base_low = 0;
    gdt.entries[2].base_mid = 0;
    gdt.entries[2].access = 0b10010010;
    gdt.entries[2].granularity = 0b11001111;
    gdt.entries[2].base_high = 0;
}

void gdt_reload() {
    struct gdt_register gdt_reg;
    gdt_reg.base = (uint32_t) &gdt;
    gdt_reg.limit = sizeof(gdt) - 1;
    asm volatile(
        "lgdt %0\n\t"
        "jmp $0x08,$1f\n\t"
        "1:\n\t"
        "mov %1, %%ds\n\t"
        "mov %1, %%es\n\t"
        "mov %1, %%gs\n\t"
        "mov %1, %%fs\n\t"
        "mov %1, %%ss\n\t"
        :: "m"(gdt_reg), "rm"((uint32_t) GDT_PM32_DS)
        : "memory"
    );
}
