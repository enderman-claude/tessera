#include "tessera/memory/pmm.h"
#include "tessera/memory/bitmap_alloc.h"
#include "tessera/memory/ram_bad_apply.h"
#include "tessera/memory/ram_bad_list.h"

/* Two bitmaps over [base, base + total pages). Bit set means:
 *   not_usable: the page is outside RAM or on the bad list. Never allocated, never freed.
 *   in_use:     the page is allocated, or not usable. bitmap_alloc_page skips set bits.
 */
static uint8_t not_usable[PMM_MAX_PAGES / 8];
static uint8_t in_use[PMM_MAX_PAGES / 8];
static uint64_t base;
static uint32_t total;
static uint32_t free_now;

static uint64_t align_up(uint64_t a)
{
    return (a + PMM_PAGE_SIZE - 1) & ~(uint64_t)(PMM_PAGE_SIZE - 1);
}

static uint64_t align_down(uint64_t a)
{
    return a & ~(uint64_t)(PMM_PAGE_SIZE - 1);
}

static int get_bit(const uint8_t *map, uint32_t i)
{
    return (map[i / 8] >> (i % 8)) & 1u;
}

static void clear_bit(uint8_t *map, uint32_t i)
{
    map[i / 8] &= (uint8_t)~(1u << (i % 8));
}

/* Span of the managed range: from the lowest region, rounded out, capped at PMM_MAX_PAGES. */
static void compute_span(const struct ram_region *regions, uint32_t n)
{
    uint64_t lo = 0, hi = 0;
    int any = 0;

    for (uint32_t i = 0; i < n; i++) {
        if (regions[i].len == 0) continue;
        uint64_t a = regions[i].addr;
        uint64_t e = a + regions[i].len;
        if (!any || a < lo) lo = a;
        if (!any || e > hi) hi = e;
        any = 1;
    }
    base = align_down(lo);
    uint64_t pages = any ? (align_up(hi) - base) / PMM_PAGE_SIZE : 0;
    total = pages > PMM_MAX_PAGES ? PMM_MAX_PAGES : (uint32_t)pages;
}

int pmm_init(const struct ram_region *regions, uint32_t n)
{
    compute_span(regions, n);
    if (total == 0) return -1;

    /* Start with everything unusable and in use, then open only the whole pages of RAM. */
    uint32_t bytes = (total + 7) / 8;
    for (uint32_t b = 0; b < bytes; b++) {
        not_usable[b] = 0xFF;
        in_use[b] = 0xFF;
    }
    uint64_t limit = base + (uint64_t)total * PMM_PAGE_SIZE;
    for (uint32_t i = 0; i < n; i++) {
        uint64_t s = align_up(regions[i].addr);
        uint64_t e = align_down(regions[i].addr + regions[i].len);
        if (regions[i].len == 0 || s < base) continue;
        if (e > limit) e = limit;
        for (uint64_t a = s; a + PMM_PAGE_SIZE <= e; a += PMM_PAGE_SIZE) {
            uint32_t idx = (uint32_t)((a - base) / PMM_PAGE_SIZE);
            clear_bit(not_usable, idx);
            clear_bit(in_use, idx);
        }
    }

    /* Bad pages from the boot RAM test are unusable, exactly like gaps. */
    ram_bad_apply(not_usable, base, total);
    ram_bad_apply(in_use, base, total);

    free_now = 0;
    for (uint32_t p = 0; p < total; p++) {
        if (!get_bit(not_usable, p) && !get_bit(in_use, p)) free_now++;
    }
    return (int)free_now;
}

uint64_t pmm_alloc_page(void)
{
    int idx = bitmap_alloc_page(in_use, total);
    if (idx < 0) return 0;
    free_now--;
    return base + (uint64_t)idx * PMM_PAGE_SIZE;
}

int pmm_free_page(uint64_t addr)
{
    if (addr < base || (addr - base) % PMM_PAGE_SIZE != 0) return -1;
    uint64_t idx = (addr - base) / PMM_PAGE_SIZE;
    if (idx >= total) return -1;
    uint32_t i = (uint32_t)idx;
    if (get_bit(not_usable, i) || !get_bit(in_use, i)) return -1;
    clear_bit(in_use, i);
    free_now++;
    return 0;
}

uint32_t pmm_free_count(void)
{
    return free_now;
}
