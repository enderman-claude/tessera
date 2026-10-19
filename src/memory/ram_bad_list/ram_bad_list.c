#include "tessera/memory/ram_bad_list.h"

static uint64_t pages[RAM_BAD_MAX];
static uint32_t count;
static uint32_t overflow;

void ram_bad_reset(void)
{
    count = 0;
    overflow = 0;
}

int ram_bad_add(uint64_t page_addr)
{
    uint64_t page = page_addr & ~(uint64_t)(RAM_PAGE - 1);
    if (ram_bad_contains(page)) {
        return 0;
    }
    if (count >= RAM_BAD_MAX) {
        overflow++;
        return -1;
    }
    pages[count++] = page;
    return 0;
}

int ram_bad_contains(uint64_t addr)
{
    uint64_t page = addr & ~(uint64_t)(RAM_PAGE - 1);
    for (uint32_t i = 0; i < count; i++) {
        if (pages[i] == page) return 1;
    }
    return 0;
}

uint32_t ram_bad_count(void)
{
    return count;
}

uint64_t ram_bad_at(uint32_t i)
{
    return pages[i];
}

uint32_t ram_bad_overflow(void)
{
    return overflow;
}
