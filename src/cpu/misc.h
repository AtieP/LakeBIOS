#ifndef __CPU_MISC_H__
#define __CPU_MISC_H__

static inline void pause() {
    __asm__ volatile("pause");
}

static inline void hlt() {
    __asm__ volatile("hlt");
}

static inline void cli() {
    __asm__ volatile("cli");
}

static inline void sti() {
    __asm__ volatile("sti");
}

#endif
