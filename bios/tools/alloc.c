#include <stdint.h>
#include <drivers/rtc.h>
#include <tools/alloc.h>
#include <tools/string.h>

#define OBJECT_SIZE 32
#define HEAP_SIZE 16384

// Simple bitmap allocator, object size is 32 bytes
#define BIT_SET(__bit) (bitmap[(__bit) / 8] |= (1 << ((__bit) % 8)))
#define BIT_CLEAR(__bit) (bitmap[(__bit) / 8] &= ~(1 << ((__bit) % 8)))
#define BIT_TEST(__bit) ((bitmap[(__bit) / 8] >> ((__bit) % 8)) & 1)

static uint8_t bitmap[HEAP_SIZE / OBJECT_SIZE / 8] = {0};
static uintptr_t alloc_base;

void alloc_setup() {
    // Reserve 16KB from low memory
    alloc_base = rtc_get_low_mem() - HEAP_SIZE;
}

void *malloc(size_t size, size_t alignment) {
    // Round size to 32
    size = (size + OBJECT_SIZE - 1) & ~(OBJECT_SIZE - 1);
    size_t pages = size / 32;
    size_t pages_found = 0;
    for (size_t i = 0; i < sizeof(bitmap) * OBJECT_SIZE; i++) {
        if (!BIT_TEST(i)) {
            if (pages_found == 0) {
                if (!(alloc_base + (OBJECT_SIZE * i) % alignment)) {
                    continue;
                }
            }
            pages_found++;
        } else if (pages_found != pages) {
            pages_found = 0;
            continue;
        }
        if (pages_found == pages) {
            size_t j;
            for (j = 0; j < pages; j++) {
                BIT_SET(i - j);
            }
            i -= j - 1;
            return (void *) (alloc_base + (OBJECT_SIZE * i));
        }
    }
    return NULL;
}

void *calloc(size_t size, size_t alignment) {
    void *ret = malloc(size, alignment);
    if (ret) {
        memset(ret, 0, size);
    }
    return ret;
}

void free(void *base, size_t size) {
    size = (size + OBJECT_SIZE - 1) & ~(OBJECT_SIZE - 1);
    size_t pages = size / OBJECT_SIZE;
    for (size_t i = 0; i < pages; i++, base += OBJECT_SIZE) {
        BIT_CLEAR(((uintptr_t) base - alloc_base) / OBJECT_SIZE);
    }
}
