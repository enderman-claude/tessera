#ifndef TESSERA_MEMORY_RAM_BAD_APPLY_H
#define TESSERA_MEMORY_RAM_BAD_APPLY_H
#include <stdint.h>

/* Marks every bad page in [base, base + pages * RAM_PAGE) as used in a bitmap allocator
 * bitmap (one bit per page, bit set = used). Lets a bitmap allocator skip bad RAM. */
void ram_bad_apply(uint8_t *bitmap, uint64_t base, uint64_t pages);

#endif
