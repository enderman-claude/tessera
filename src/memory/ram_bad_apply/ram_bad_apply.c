#include "tessera/memory/ram_bad_apply.h"
#include "tessera/memory/ram_bad_list.h"

void ram_bad_apply(uint8_t *bitmap, uint64_t base, uint64_t pages)
{
    uint64_t end = base + pages * RAM_PAGE;
    for (uint32_t i = 0; i < ram_bad_count(); i++) {
        uint64_t a = ram_bad_at(i);
        if (a < base || a >= end) continue;
        uint64_t idx = (a - base) / RAM_PAGE;
        bitmap[idx / 8] |= (uint8_t)(1u << (idx % 8));
    }
}
