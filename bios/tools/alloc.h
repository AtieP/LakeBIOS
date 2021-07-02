#ifndef __TOOLS_ALLOC_H__
#define __TOOLS_ALLOC_H__

#include <stddef.h>

void alloc_setup();
void *malloc(size_t size, size_t alignment);
void *calloc(size_t size, size_t alignment);
void free(void *base, size_t size);

#endif
