#ifndef __TOOLS_WAIT_H__
#define __TOOLS_WAIT_H__

#include <stddef.h>
#include <cpu/pio.h>

static inline void wait(size_t count) {
    for (size_t i = 0; i < count; i++) {
        inb(0x80);
    }
}

#endif
