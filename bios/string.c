#include <stdint.h>
#include <string.h>

void *memcpy(void *dest, const void *src, size_t n) {
    const uint8_t *src_uint8 = (const uint8_t *) src;
    uint8_t *dest_uint8 = (uint8_t *) dest;
    for (size_t i = 0; i < n; i++) {
        dest_uint8[i] = src_uint8[i];
    }
    return dest;
}
