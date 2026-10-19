/* Host tests for raw payload injection. The "RAM" is an mmap'd page-aligned region, and the
 * fake memory map marks it as conventional memory. Payload code is real x86-64. */
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "tessera/boot/payload_check.h"
#include "tessera/boot/payload_run.h"

static int failures;
#define CHECK(cond, name) do { if (cond) printf("ok   %s\n", name); else { printf("FAIL %s\n", name); failures++; } } while (0)

#define REGION (2 * 4096)
static uint8_t *region;
static EFI_MEMORY_DESCRIPTOR desc[2];
static struct efi_memmap map;

/* mov eax, 42; ret */
static const uint8_t code[] = { 0xB8, 0x2A, 0x00, 0x00, 0x00, 0xC3 };

static void put32(uint8_t *p, uint32_t v) { for (int b = 0; b < 4; b++) p[b] = (uint8_t)(v >> (8 * b)); }

static void build(uint32_t size, uint32_t entry, uint32_t sum_override, int have_magic)
{
    memset(region, 0, REGION);
    if (have_magic) { region[0] = 'T'; region[1] = 'I'; region[2] = 'N'; region[3] = 'J'; }
    region[4] = PAYLOAD_VERSION;
    put32(region + 8, size);
    put32(region + 12, entry);
    memcpy(region + PAYLOAD_DESC_SIZE, code, sizeof code);
    uint32_t sum = 0;
    for (uint32_t i = 0; i < size && i < sizeof code; i++) sum += code[i];
    put32(region + 16, sum_override ? sum_override : sum);
}

static void map_as(uint32_t type, uint64_t pages)
{
    memset(desc, 0, sizeof desc);
    desc[0].Type = type;
    desc[0].PhysicalStart = (uint64_t)(uintptr_t)region;
    desc[0].NumberOfPages = pages;
    map.buf = (uint8_t *)desc;
    map.buf_size = sizeof desc;
    map.used = sizeof desc[0];
    map.desc_size = sizeof desc[0];
}

static void test_payload(void)
{
    uint64_t entry = 0;
    build((uint32_t)sizeof code, 0, 0, 1);
    map_as(EFI_CONVENTIONAL_MEMORY, 2);
    CHECK(payload_check((uint64_t)(uintptr_t)region, &map, &entry) == PAYLOAD_OK, "payload: valid payload accepted");
    int (*fn)(void) = (int (*)(void))(uintptr_t)entry;
    CHECK(fn() == 42, "payload: injected code runs and returns 42");

    build((uint32_t)sizeof code, 0, 0, 0);
    CHECK(payload_check((uint64_t)(uintptr_t)region, &map, &entry) == PAYLOAD_NO_MAGIC, "payload: no magic");

    build((uint32_t)sizeof code, 0, 0, 1);
    region[4] = 9;
    CHECK(payload_check((uint64_t)(uintptr_t)region, &map, &entry) == PAYLOAD_BAD_VERSION, "payload: bad version");

    build((uint32_t)sizeof code, 0, 0, 1);
    region[20] = 1;
    CHECK(payload_check((uint64_t)(uintptr_t)region, &map, &entry) == PAYLOAD_BAD_FLAGS, "payload: nonzero flags");

    build(0, 0, 0, 1);
    CHECK(payload_check((uint64_t)(uintptr_t)region, &map, &entry) == PAYLOAD_BAD_SIZE, "payload: zero size");

    build((uint32_t)sizeof code, 99, 0, 1);
    CHECK(payload_check((uint64_t)(uintptr_t)region, &map, &entry) == PAYLOAD_BAD_ENTRY, "payload: entry outside payload");

    build((uint32_t)sizeof code, 0, 7, 1);
    CHECK(payload_check((uint64_t)(uintptr_t)region, &map, &entry) == PAYLOAD_BAD_CHECKSUM, "payload: checksum mismatch");

    build((uint32_t)sizeof code, 0, 0, 1);
    map_as(EFI_CONVENTIONAL_MEMORY, 1);
    /* payload is 38 bytes long, so keep it inside page 0 but make page 0 too small */
    desc[0].NumberOfPages = 0;
    CHECK(payload_check((uint64_t)(uintptr_t)region, &map, &entry) == PAYLOAD_NOT_RAM, "payload: no conventional RAM covering it");

    build((uint32_t)sizeof code, 0, 0, 1);
    map_as(7 + 1, 2);  /* type 8 = reserved, not conventional */
    CHECK(payload_check((uint64_t)(uintptr_t)region, &map, &entry) == PAYLOAD_NOT_RAM, "payload: reserved memory is not RAM");
}

static void test_payload_run(void)
{
    build((uint32_t)sizeof code, 0, 0, 1);
    map_as(EFI_CONVENTIONAL_MEMORY, 2);
    uint64_t entry = 0;
    CHECK(payload_check((uint64_t)(uintptr_t)region, &map, &entry) == PAYLOAD_OK, "run: payload accepted before run");
    payload_run(entry);
    CHECK(1, "run: payload_run returns after the payload returns");
}

int main(void)
{
    region = mmap(NULL, REGION, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED) {
        printf("FAIL cannot map region\n");
        return 1;
    }
    test_payload();
    test_payload_run();
    printf("%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
