#include <apis/bios/handlers.h>
#include <cpu/pio.h>
#include <cpu/smm.h>
#include <tools/print.h>
#include <tools/string.h>

__attribute__((__section__(".smm_stack"), __used__))
static uint8_t smm_stack[4096] = {0};

static uint32_t smbase = SMM_DEFAULT_SMBASE;

__attribute__((__section__(".smm_entry"), __used__))
static void smm_handler_main() {
    struct smm_state *state = (struct smm_state *) (smbase + SMM_SMBASE_STATE_OFFSET);
    uint8_t command = inb(0xb2);
    uint8_t data = inb(0xb3);
    uint32_t revision = state->regs32.smrev & 0x2ffff;
    print("lakebios: smm: Hello from SMM! Command: %d, Data: %d", command, data);
    if (command == 0x01) {
        // Command 0x01 for lakebios: Move SMBASE to 0xa0000
        smbase = SMM_NEW_SMBASE;
        if (revision == SMM_REV_32) {
            // 32-bit
            state->regs32.smbase = smbase;
        } else if (revision == SMM_REV_64) {
            // 64-bit
            state->regs64.smbase = smbase;
        } else {
            print("Invalid revision?");
            for (;;) {}
        }
    }
    if (command == 0x10) {
        // Command 0x02 for lakebios: real mode interrupt
        struct apis_bios_regs regs;
        asm volatile("mov %%cr2, %0" : "=r"(regs.eax));
        print("lakebios: smm: int %xh, ah: %x", data, (uint8_t) (regs.eax >> 8));
        if (data == 0x10) {
            apis_bios_int10(&regs);
        }
    }
    asm volatile("rsm");
    for (;;) {}
}
