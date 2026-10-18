#ifndef TESSERA_MEMORY_BITMAP_ALLOC_H
#define TESSERA_MEMORY_BITMAP_ALLOC_H
#include <stddef.h>
#include <stdint.h>

int bitmap_alloc_page(uint8_t *bitmap, size_t pages);
void bitmap_free_page(uint8_t *bitmap, size_t page);

#endif
