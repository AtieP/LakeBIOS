#include <cpu/pio.h>
#include <cpu/smm.h>
#include <tools/print.h>
#include <tools/string.h>

static uint32_t smbase = SMM_DEFAULT_SMBASE;

__attribute__((__section__(".smm_main_code"), __used__))
static void smm_handler_main() {
    struct smm_state *registers = (struct smm_state *) (smbase + SMM_SMBASE_STATE_OFFSET);
    uint8_t command = inb(0xb2);
    uint32_t revision = registers->registers.regs32.smrev & 0x2ffff;
    print("atiebios: smm: Hello from SMM! Command: %d", command);
    if (command == 0x01) {
        // Command 0x01 for AtieBIOS: Move SMBASE to 0xa0000
        smbase = SMM_NEW_SMBASE;
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
