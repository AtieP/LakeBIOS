#ifndef __CPU_MSR_H__
#define __CPU_MSR_H__

#include <stdint.h>

static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t eax;
    uint32_t edx;
    __asm__ volatile("rdmsr" : "=a"(eax), "=d"(edx) : "c"(msr));
    return ((uint64_t) edx << 32) | eax;
}

static inline void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t eax = (uint32_t) value;
    uint32_t edx = (uint32_t) (value >> 32);
    __asm__ volatile("wrmsr" :: "a"(eax), "d"(edx), "c"(msr));
}

#endif
