/* Host tests for the boot RAM test: pattern fill, bad-byte detection, scan exclusions, and
 * marking bad pages in a bitmap. The "RAM" is an mmap'd region. */
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "tessera/memory/ram_test.h"
#include "tessera/memory/ram_bad_list.h"
#include "tessera/memory/ram_bad_apply.h"
#include "tessera/boot/ram_scan.h"

static int failures;
#define CHECK(cond, name) do { if (cond) printf("ok   %s\n", name); else { printf("FAIL %s\n", name); failures++; } } while (0)

#define PAGES 4
#define LEN (PAGES * 4096)
static uint8_t *ram;

/* Simulates a stuck-at-zero byte: sets one byte to 0x00 after the write pass. */
static size_t stuck_offset;
static void stuck_hook(volatile uint8_t *base, uint64_t len, void *ctx)
{
    (void)len; (void)ctx;
    base[stuck_offset] = 0x00;
}

static void test_ram_test(void)
{
    struct ram_test_result r;
    ram_bad_reset();
    memset(ram, 0x11, LEN);
    CHECK(ram_test_range((uint64_t)(uintptr_t)ram, LEN, RAM_TEST_PATTERN, 0, 0, &r) == 0, "ram: clean range runs");
    CHECK(r.pages_tested == PAGES && r.pages_bad == 0 && r.bytes_bad == 0, "ram: clean range has no bad bytes");
    CHECK(ram_bad_count() == 0, "ram: clean range maps nothing");

    ram_bad_reset();
    stuck_offset = 4096 * 2 + 100;   /* byte 100 of page 2 */
    CHECK(ram_test_range((uint64_t)(uintptr_t)ram, LEN, RAM_TEST_PATTERN, stuck_hook, 0, &r) == 0, "ram: faulty range runs");
    CHECK(r.bytes_bad == 1 && r.pages_bad == 1, "ram: one bad byte counted once, in one page");
    CHECK(ram_bad_count() == 1, "ram: bad page recorded");
    CHECK(ram_bad_at(0) == (uint64_t)(uintptr_t)ram + 4096 * 2, "ram: the recorded page is page 2");
    CHECK(ram_bad_contains((uint64_t)(uintptr_t)ram + 4096 * 2 + 4000), "ram: any address in the page is bad");
    CHECK(!ram_bad_contains((uint64_t)(uintptr_t)ram + 4096), "ram: neighbouring page is good");
    CHECK(r.first_count == 1 && r.first[0].read == 0x00 && r.first[0].wrote == RAM_TEST_PATTERN,
          "ram: first bad byte reports wrote and read values");

    CHECK(ram_test_range((uint64_t)(uintptr_t)ram + 1, 4096, RAM_TEST_PATTERN, 0, 0, &r) != 0,
          "ram: unaligned address rejected");
}

static void test_bitmap(void)
{
    /* 4 pages starting at ram; page 2 bad (from the previous test) */
    uint8_t bitmap[1] = {0};
    ram_bad_apply(bitmap, (uint64_t)(uintptr_t)ram, PAGES);
    CHECK(bitmap[0] == (1u << 2), "bitmap: bad page marked used, others untouched");
}

static void test_scan(void)
{
    EFI_MEMORY_DESCRIPTOR d[4];
    memset(d, 0, sizeof d);
    d[0].Type = EFI_CONVENTIONAL_MEMORY; d[0].PhysicalStart = 0x100000; d[0].NumberOfPages = 16;
    d[1].Type = EFI_CONVENTIONAL_MEMORY; d[1].PhysicalStart = 0x200000; d[1].NumberOfPages = 8;   /* holds payload */
    d[2].Type = 2;                       d[2].PhysicalStart = 0x400000; d[2].NumberOfPages = 4;   /* loader data: not conventional */
    d[3].Type = EFI_CONVENTIONAL_MEMORY; d[3].PhysicalStart = 0x800000; d[3].NumberOfPages = 2;   /* holds kernel */
    struct efi_memmap m = { .buf = (uint8_t *)d, .buf_size = sizeof d, .used = sizeof d, .desc_size = sizeof d[0] };

    uint64_t keep[2] = { 0x200000 + 0x100, 0x800000 + 0x10 };
    struct ram_region out[8];
    uint32_t n = ram_scan(&m, keep, 2, out, 8);
    CHECK(n == 1, "scan: one region left after excluding kernel and payload descriptors");
    CHECK(n == 1 && out[0].addr == 0x100000 && out[0].len == 16 * 4096, "scan: the free descriptor is returned whole");

    uint64_t nokeep_n = ram_scan(&m, 0, 0, out, 8);
    CHECK(nokeep_n == 3, "scan: with no keep list, all conventional descriptors are returned");
}

int main(void)
{
    ram = mmap(NULL, LEN, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ram == MAP_FAILED) {
        printf("FAIL cannot map ram\n");
        return 1;
    }
    /* align the test region to a page boundary */
    uintptr_t a = ((uintptr_t)ram + 4095) & ~(uintptr_t)4095;
    ram = (uint8_t *)a;
    test_ram_test();
    test_bitmap();
    test_scan();
    printf("%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
