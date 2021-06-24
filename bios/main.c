#include <print.h>
#include <cpu/gdt.h>
#include <drivers/dram.h>
#include <drivers/lpc.h>

#include <string.h>

extern char smm_code_start[];
extern char smm_code_end[];

#include <cpu/pio.h>

__attribute__((__section__(".smm"), used, __naked__))
static void smm_entry() {
    outb(0xe9, 'H'); outb(0xe9, 'e'); outb(0xe9, 'l'); outb(0xe9, 'l'); outb(0xe9, 'o');
    outb(0xe9, ' ');
    outb(0xe9, 'f'); outb(0xe9, 'r'); outb(0xe9, 'o'); outb(0xe9, 'm');
    outb(0xe9, ' ');
    outb(0xe9, 'S'); outb(0xe9, 'M'); outb(0xe9, 'M'); outb(0xe9, '!'); 
    asm volatile("rsm");
}

__attribute__((__section__(".c_init"), used))
void bios_main() {
    dram_unlock_bios();
    gdt_craft();
    gdt_reload();
    dram_enable_smram();
    dram_open_smram();
    // Todo: repolish all the things below, should relocate SMM handler to 0xa8000
    memcpy((void *) 0x38000, smm_code_start, smm_code_end - smm_code_start);
    dram_close_smram();
    dram_lock_smram();
    lpc_set_acpi_base(0x600);
    lpc_enable_acpi();
    // ACPI SMI control block, this right now enables SMIs (bit 0) and generation of SMIs when writing to the APM control port (bit 5)
    outd(0x600 + 0x30, ind(0x600 + 0x30) | 1 | (1 << 5));
    outb(0xb2, 0);// APM control port
    for (;;);
}
