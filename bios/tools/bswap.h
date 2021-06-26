#ifndef __TOOLS_BSWAP_H__
#define __TOOLS_BSWAP_H__

#include <stdint.h>

static inline uint16_t bswap16(uint16_t number) {
    return (number << 8) | (number >> 8);
}

static inline uint32_t bswap32(uint32_t number) {
    return ((number << 24) & 0xff000000) | ((number << 8) & 0x00ff0000) | ((number >> 8) & 0x0000ff00) | ((number >> 24) & 0x000000ff);
}

static inline uint64_t bswap64(uint64_t number) {
    uint32_t h = bswap32(number & 0xffffffff);
    uint32_t l = bswap32((number >> 32) & 0xffffffff);
    return ((uint64_t) h << 32) | l;
}

#endif
