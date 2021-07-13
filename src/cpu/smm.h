#ifndef __CPU_SMM_H__
#define __CPU_SMM_H__

#include <stdint.h>

#define SMM_DEFAULT_SMBASE 0x30000
#define SMM_NEW_SMBASE 0xa0000
#define SMM_SMBASE_HANDLER_OFFSET 0x8000
#define SMM_SMBASE_STATE_OFFSET 0xfe00

#define SMM_REV_32 0x20000
#define SMM_REV_64 0x20064

extern char smm_trampoline_start[];
extern char smm_trampoline_end[];

// Most of the "reserved" registers here aren't actually reserved.
// Apparently, the SMM layout between Intel and AMD processors differ.
struct smm_state {
    union {
        struct {
            char reserved1[0xf8];
            uint32_t smbase;
            uint32_t smrev;
            char reserved2[0xd0];
            uint32_t eax;
            uint32_t ecx;
            uint32_t edx;
            uint32_t ebx;
            uint32_t esp;
            uint32_t ebp;
            uint32_t esi;
            uint32_t edi;
            uint32_t eip;
            uint32_t eflags;
            char reserved3[0x08];
        } __attribute__((__packed__)) regs32;
        struct {
            char reserved1[0xfc];
            uint32_t smrev;
            uint32_t smbase;
            char reserved2[0x6c];
            uint64_t rflags;
            uint64_t rip;
            uint64_t r15;
            uint64_t r14;
            uint64_t r13;
            uint64_t r12;
            uint64_t r11;
            uint64_t r10;
            uint64_t r9;
            uint64_t r8;
            uint64_t rdi;
            uint64_t rsi;
            uint64_t rbp;
            uint64_t rsp;
            uint64_t rbx;
            uint64_t rdx;
            uint64_t rcx;
            uint64_t rax;
        } __attribute__((__packed__)) regs64;
    };
} __attribute__((__packed__));

#endif
