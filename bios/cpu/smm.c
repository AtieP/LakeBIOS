#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/dram.h>
#include <drivers/lpc.h>
#include <tools/print.h>
#include <tools/string.h>

extern char smm_entry_code_start[];
extern char smm_entry_code_end[];
extern char smm_main_code_start[];
extern char smm_main_code_end[];

__attribute__((__section__(".smm_main_code"), __used__))
static void smm_main() {
    print("atiebios: smm: Hello from SMM!");
    asm volatile("rsm");
    for (;;) {}
}

void smm_init() {
    // Place the SMM code
    dram_enable_smram();
    dram_open_smram();
    memcpy((void *) 0x38000, smm_entry_code_start, smm_entry_code_end - smm_entry_code_start);
    memcpy((void *) 0x38000 + (smm_entry_code_end - smm_entry_code_start), smm_main_code_start, smm_main_code_end - smm_main_code_start);
    dram_close_smram();
    dram_lock_smram();
    // Enable ACPI registers to be able to access the SMI register
    lpc_set_acpi_base(0x600);
    lpc_enable_acpi();
    outd(0x600 + 0x30, ind(0x600 + 0x30) | (1 << 5) | 1);
    // Test
    outb(0xb2, 0);
}
