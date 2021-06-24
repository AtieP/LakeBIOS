#include <cpu/pio.h>
#include <cpu/smm.h>
#include <drivers/dram.h>
#include <drivers/lpc.h>
#include <tools/print.h>
#include <tools/string.h>

static uint32_t smbase = SMM_DEFAULT_SMBASE;

extern char smm_entry_code_start[];
extern char smm_entry_code_end[];

__attribute__((__section__(".smm_main_code"), __used__))
static void smm_main() {
    struct smm_state *registers = (struct smm_state *) (smbase + SMM_SMBASE_STATE_OFFSET);
    uint8_t command = inb(0xb2);
    uint32_t revision = registers->registers.regs32.smrev & 0x2ffff;
    print("atiebios: smm: Hello from SMM! Command: %d", command);
    if (command == 0x01) {
        // Command 0x01 for AtieBIOS: Move SMBASE to 0xa0000
        smbase = SMM_Q35_SMBASE;
        if (revision == SMM_REV_32) {
            // 32-bit
            registers->registers.regs32.smbase = smbase;
        } else if (revision == SMM_REV_64) {
            // 64-bit
            registers->registers.regs64.smbase = smbase;
        } else {
            print("Invalid revision?");
            for (;;) {}
        }
    }
    asm volatile("rsm");
    for (;;) {}
}

void smm_init() {
    // Place the SMM code
    dram_enable_smram();
    dram_open_smram();
    memcpy((void *) SMM_DEFAULT_SMBASE + SMM_SMBASE_HANDLER_OFFSET, smm_entry_code_start, smm_entry_code_end - smm_entry_code_start);
    memcpy((void *) SMM_Q35_SMBASE + SMM_SMBASE_HANDLER_OFFSET, smm_entry_code_start, smm_entry_code_end - smm_entry_code_start);
    dram_close_smram();
    dram_lock_smram();
    // Enable ACPI registers to be able to access the SMI register
    lpc_set_acpi_base(0x600);
    lpc_enable_acpi();
    // Enable SMIs in general, and SMIs when writing to APM registers (0xb2, 0xb3)
    outd(0x600 + 0x30, ind(0x600 + 0x30) | (1 << 5) | 1);
    // Set SMBASE to 0xa0000 because the (G)MCH will shadow it unlike with 0x30000
    outb(0xb2, 0x01);
    // Clear old SMBASE
    memset((void *) SMM_DEFAULT_SMBASE, 0, 0x10000);
}
