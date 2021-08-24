#ifndef __CPU_PIO_H__
#define __CPU_PIO_H__

#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("in %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile("in %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint32_t ind(uint16_t port) {
    uint32_t ret;
    __asm__ volatile("in %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("out %0, %1" :: "a"(data), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t data) {
    __asm__ volatile("out %0, %1" :: "a"(data), "Nd"(port));
}

static inline void outd(uint16_t port, uint32_t data) {
    __asm__ volatile("out %0, %1" :: "a"(data), "Nd"(port));
}

#endif