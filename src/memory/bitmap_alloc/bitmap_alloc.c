#include "tessera/memory/bitmap_alloc.h"

/* One bit per page. A set bit means the page is in use. */
int bitmap_alloc_page(uint8_t *bitmap, size_t pages)
{
    for (size_t i = 0; i < pages; i++) {
        uint8_t mask = (uint8_t)(1u << (i % 8));
        if (!(bitmap[i / 8] & mask)) {
            bitmap[i / 8] |= mask;
            return (int)i;
        }
    }
    return -1;
}

void bitmap_free_page(uint8_t *bitmap, size_t page)
{
    bitmap[page / 8] &= (uint8_t)~(1u << (page % 8));
}
