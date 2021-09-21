#include <cpu/pio.h>
#include <cpu/smm.h>
#include <tools/print.h>

__attribute__((__section__(".smm_stack"), __used__))
static uint8_t smm_stack[4096] = {0};

static uint32_t smbase = SMM_DEFAULT_SMBASE;

__attribute__((__section__(".smm_entry"), __used__))
static void smm_handler_main() {
    struct smm_state *state = (struct smm_state *) (smbase + SMM_SMBASE_STATE_OFFSET);
    uint8_t command = inb(0xb2);
    uint8_t data = inb(0xb3);
    uint32_t revision = state->regs32.smrev & 0x2ffff;
    if (command == 0x01) {
        // Command 0x01 for lakebios: Move SMBASE to 0xa0000
        print("SMM: Moving SMBASE to 0xa0000");
        // If SMBASE is already 0xa0000 then the revision is 0
        smbase = SMM_NEW_SMBASE;
        if (revision) {
            if (revision == SMM_REV_32) {
                state->regs32.smbase = smbase;
            } else if (revision == SMM_REV_64) {
                state->regs64.smbase = smbase;
            } else {
                print("SMM: Invalid SMM revision");
                for (;;) {}
            }
        }
    }
    if (command == 0x10) {
        // Command 0x10 for lakebios: Real mode interrupt
    }
    __asm__ volatile("rsm");
    for (;;) {}
}
