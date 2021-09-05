#ifndef __TOOLS_ALLOC_H__
#define __TOOLS_ALLOC_H__

#include <stddef.h>

#define HEAP_SIZE (16384 * 4)

void alloc_setup(uintptr_t base);
void *malloc(size_t size, size_t alignment);
void *realloc(void *old, size_t oldsize, size_t newsize, size_t alignment);
void *calloc(size_t size, size_t alignment);
void free(void *base, size_t size);

#endif
