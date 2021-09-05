#include <stdint.h>
#include <tools/alloc.h>
#include <tools/print.h>
#include <tools/string.h>

#define OBJECT_SIZE 32

// Simple bitmap allocator, object size is 32 bytes
#define BIT_SET(__bit) (bitmap[(__bit) / 8] |= (1 << ((__bit) % 8)))
#define BIT_CLEAR(__bit) (bitmap[(__bit) / 8] &= ~(1 << ((__bit) % 8)))
#define BIT_TEST(__bit) ((bitmap[(__bit) / 8] >> ((__bit) % 8)) & 1)

static uint8_t bitmap[HEAP_SIZE / OBJECT_SIZE / 8] = {0};
static uintptr_t alloc_base;

void alloc_setup(uintptr_t base) {
    // Reserve 64KB from low memory
    alloc_base = base;
}

void *malloc(size_t size, size_t alignment) {
    if (!size) {
        print("alloc: tried to allocate a zone with a 0 size, ignoring");
        return NULL;
    }
    // Round size to 32
    if (size % 32) {
        size = (size + OBJECT_SIZE - 1) & ~(OBJECT_SIZE - 1);
    }
    size_t pages = size / 32;
    size_t pages_found = 0;
    for (size_t i = 0; i < sizeof(bitmap) * 8; i++) {
        if (!BIT_TEST(i)) {
            if (pages_found == 0) {
                if ((alloc_base + (OBJECT_SIZE * i)) % alignment) {
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

void *realloc(void *old, size_t oldsize, size_t newsize, size_t alignment) {
    if (!old && !newsize) {
        return NULL;
    }
    if (!newsize) {
        free(old, oldsize);
        return NULL;
    }
    if (!old) {
        return malloc(newsize, alignment);
    }
    void *ret = malloc(newsize, alignment);
    if (!ret) {
        return NULL;
    }
    memcpy(ret, old, oldsize);
    free(old, oldsize);
    return ret;
}

void *calloc(size_t size, size_t alignment) {
    void *ret = malloc(size, alignment);
    if (ret) {
        memset(ret, 0, size);
    }
    return ret;
}

void free(void *base, size_t size) {
    if (!base) {
        print("alloc: SEVERE WARNING: trying to free a NULL pointer!!!");
        return;
    }
    uintptr_t base_int = (uintptr_t) base;
    if (!size) {
        print("alloc: tried to free a zone with 0 size, ignoring");
        return;
    }
    if (size % 32) {
        size = (size + OBJECT_SIZE - 1) & ~(OBJECT_SIZE - 1);
    }
    size_t pages = size / OBJECT_SIZE;
    for (size_t i = 0; i < pages; i++, base_int += OBJECT_SIZE) {
        BIT_CLEAR((base_int - alloc_base) / OBJECT_SIZE);
    }
}
