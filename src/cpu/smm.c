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
    if (command == 0x01) {
        // Command 0x01 for lakebios: Move SMBASE to 0xa0000
        print("lakebios: smm: moving SMBASE to 0xa0000");
        // If SMBASE is already 0xa0000 then the revision is 0
        smbase = SMM_NEW_SMBASE;
        if (revision) {
            if (revision == SMM_REV_32) {
                state->regs32.smbase = smbase;
            } else if (revision == SMM_REV_64) {
                state->regs64.smbase = smbase;
            } else {
                print("lakebios: smm: invalid revision. Halting.");
                for (;;) {}
            }
        }
    }
    if (command == 0x10) {
        // Command 0x10 for lakebios: real mode interrupt
        // Create the register state
        struct apis_bios_regs regs;
        if (revision == SMM_REV_32) {
            regs.ebx = state->regs32.ebx;
            regs.ecx = state->regs32.ecx;
            regs.edx = state->regs32.edx;
            regs.esi = state->regs32.esi;
            regs.edi = state->regs32.edi;
            regs.ebp = state->regs32.ebp;
            regs.eflags = state->regs32.eflags;
            regs.es = *((uint32_t *) (smbase + 0xffa8)) << 4;
            regs.ds = *((uint32_t *) (smbase + 0xffb4)) << 4;
        } else if (revision == SMM_REV_64) {
            regs.ebx = state->regs64.rbx;
            regs.ecx = state->regs64.rcx;
            regs.edx = state->regs64.rdx;
            regs.esi = state->regs64.rsi;
            regs.edi = state->regs64.rdi;
            regs.ebp = state->regs64.rbp;
            regs.eflags = state->regs64.rflags;
            regs.es = state->regs64.es.base;
            regs.ds = state->regs64.ds.base;
        } else {
            print("lakebios: smm: invalid revision. Halting.");
            for (;;) {}
        }
        asm volatile("mov %%cr2, %0" : "=r"(regs.eax));
        print("lakebios: smm: real mode int %xh ah %xh", data, (uint8_t) (regs.eax >> 8));
        if (data == 0x10) {
            apis_bios_int10(&regs);
        } else if (data == 0x13) {
            apis_bios_int13(&regs);
        } else if (data == 0x16) {
            apis_bios_int16(&regs);
        } else if (data == 0x19) {
            apis_bios_int19(&regs);
        } else {
            print("lakebios: bios: handler not available.");
            for (;;) {}
        }
        if (revision == SMM_REV_32) {
            state->regs32.eax = regs.eax;
            state->regs32.ebx = regs.ebx;
            state->regs32.ecx = regs.ecx;
            state->regs32.edx = regs.edx;
            state->regs32.esi = regs.esi;
            state->regs32.edi = regs.edi;
            state->regs32.ebp = regs.ebp;
            state->regs32.eflags = regs.eflags;
        } else if (revision == SMM_REV_64) {
            state->regs64.rax &= ~0xffffffff;
            state->regs64.rax |= regs.eax;
            state->regs64.rbx &= ~0xffffffff;
            state->regs64.rbx |= regs.ebx;
            state->regs64.rcx &= ~0xffffffff;
            state->regs64.rcx |= regs.ecx;
            state->regs64.rdx &= ~0xffffffff;
            state->regs64.rdx |= regs.edx;
            state->regs64.rsi &= ~0xffffffff;
            state->regs64.rsi |= regs.esi;
            state->regs64.rdi &= ~0xffffffff;
            state->regs64.rdi |= regs.edi;
            state->regs64.rbp &= ~0xffffffff;
            state->regs64.rbp |= regs.ebp;
            state->regs64.rflags &= ~0xffffffff;
            state->regs64.rflags |= regs.eflags;
        } else {
            print("lakebios: smm: invalid revision. Halting.");
            for (;;) {}
        }
    }
    asm volatile("rsm");
    for (;;) {}
}
