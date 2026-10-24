/* Host tests for the page allocator. Regions are fake physical addresses. The allocator never
 * reads or writes the memory it manages, so no real RAM is needed. */
#include <stdio.h>
#include "tessera/memory/pmm.h"
#include "tessera/memory/ram_bad_list.h"

static int failures;
#define CHECK(cond, name) do { if (cond) printf("ok   %s\n", name); else { printf("FAIL %s\n", name); failures++; } } while (0)

#define P PMM_PAGE_SIZE
/* Two regions with a gap between them: 4 pages at 0x100000, 2 pages at 0x200000. */
static const struct ram_region regions[] = {
    { 0x100000, 4 * P },
    { 0x200000, 2 * P },
};
#define NREG (sizeof regions / sizeof regions[0])

static int in_regions(uint64_t a)
{
    for (unsigned i = 0; i < NREG; i++) {
        if (a >= regions[i].addr && a < regions[i].addr + regions[i].len) return 1;
    }
    return 0;
}

static void test_init_and_alloc(void)
{
    ram_bad_reset();
    CHECK(pmm_init(regions, NREG) == 6, "pmm: six free pages in two regions");
    CHECK(pmm_free_count() == 6, "pmm: free count matches init");

    uint64_t got[6];
    int ok = 1;
    for (int i = 0; i < 6; i++) {
        got[i] = pmm_alloc_page();
        if (got[i] == 0 || !in_regions(got[i]) || (got[i] % P) != 0) ok = 0;
        for (int j = 0; j < i; j++) if (got[j] == got[i]) ok = 0;
    }
    CHECK(ok, "pmm: six allocations, all page-aligned, inside RAM, all distinct");
    CHECK(pmm_free_count() == 0, "pmm: free count reaches zero");
    CHECK(pmm_alloc_page() == 0, "pmm: exhausted allocator returns 0");
}

static void test_gap_never_handed_out(void)
{
    ram_bad_reset();
    pmm_init(regions, NREG);
    /* The gap between the regions is at 0x104000..0x200000. Nothing from there is handed out. */
    int in_gap = 0;
    for (int i = 0; i < 6; i++) {
        uint64_t a = pmm_alloc_page();
        if (a >= 0x104000 && a < 0x200000) in_gap = 1;
    }
    CHECK(!in_gap, "pmm: pages in the gap between regions are never allocated");
}

static void test_free_and_reuse(void)
{
    ram_bad_reset();
    pmm_init(regions, NREG);
    uint64_t a = pmm_alloc_page();
    CHECK(pmm_free_page(a) == 0, "pmm: freeing an allocated page succeeds");
    CHECK(pmm_free_count() == 6, "pmm: free count restored after free");
    CHECK(pmm_free_page(a) == -1, "pmm: double free is rejected");
    CHECK(pmm_free_page(0x104000) == -1, "pmm: freeing a gap page is rejected");
    CHECK(pmm_free_page(0x100000 + 1) == -1, "pmm: unaligned address is rejected");
    CHECK(pmm_free_page(0x300000) == -1, "pmm: address outside the managed range is rejected");
}

static void test_bad_page_excluded(void)
{
    ram_bad_reset();
    ram_bad_add(0x101000);   /* second page of the first region */
    CHECK(pmm_init(regions, NREG) == 5, "pmm: a bad page reduces the free count");
    int handed_out_bad = 0;
    for (int i = 0; i < 5; i++) {
        if (pmm_alloc_page() == 0x101000) handed_out_bad = 1;
    }
    CHECK(!handed_out_bad, "pmm: a bad page is never handed out");
    CHECK(pmm_free_page(0x101000) == -1, "pmm: a bad page cannot be freed into the pool");
}

static void test_no_ram(void)
{
    ram_bad_reset();
    CHECK(pmm_init(regions, 0) == -1, "pmm: no regions is an error");
}

int main(void)
{
    test_init_and_alloc();
    test_gap_never_handed_out();
    test_free_and_reuse();
    test_bad_page_excluded();
    test_no_ram();
    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("PASSED (0 failures)\n");
    return 0;
}
