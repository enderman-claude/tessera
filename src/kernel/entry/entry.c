#include "tessera/kernel/entry.h"
#include "tessera/kernel/halt.h"
#include "tessera/boot/ram_scan.h"
#include "tessera/boot/payload_check.h"
#include "tessera/boot/payload_run.h"
#include "tessera/memory/ram_test.h"
#include "tessera/memory/ram_bad_list.h"
#include "tessera/console/serial_com1.h"
#include "tessera/lib/u64_to_dec.h"
#include "tessera/shell/serial_root.h"
#include "tessera/trap/init.h"
#include "tessera/kernel/panic.h"

#define MAX_REGIONS 512

static struct ram_region regions[MAX_REGIONS];

static void fatal_to_panic(const struct trap_frame *f)
{
    (void)f;
    kernel_panic();
}

void kernel_main(const struct efi_memmap *m, const uint64_t *keep, uint32_t n_keep,
                 int payload_ready, uint64_t payload_entry)
{
    /* Exceptions go to the Morse panic from here on. */
    trap_init(fatal_to_panic);

    char num[24];
    uint32_t nreg = ram_scan(m, keep, n_keep, regions, MAX_REGIONS);
    struct ram_test_result total = {0};
    ram_bad_reset();

    /* Fill every free RAM region with each test pattern, read it back, and mark bad pages.
     * A page that fails either pattern is counted once, by the bad-page list. */
    static const uint8_t patterns[RAM_TEST_PATTERN_COUNT] = { RAM_TEST_PATTERN, RAM_TEST_PATTERN_2 };
    for (uint32_t p = 0; p < RAM_TEST_PATTERN_COUNT; p++) {
        for (uint32_t r = 0; r < nreg; r++) {
            struct ram_test_result one;
            if (ram_test_range(regions[r].addr, regions[r].len, patterns[p], 0, 0, &one) == 0) {
                if (p == 0) total.pages_tested += one.pages_tested;
                total.bytes_bad += one.bytes_bad;
            }
        }
    }
    total.pages_bad = ram_bad_count();

    serial_com1_puts("tessera: ram test patterns 0x42 0xBD over ");
    u64_to_dec(total.pages_tested, num);
    serial_com1_puts(num);
    serial_com1_puts(" pages\n");
    u64_to_dec(total.pages_bad, num);
    serial_com1_puts("tessera: bad pages mapped: ");
    serial_com1_puts(num);
    serial_com1_puts("\n");
    u64_to_dec(total.bytes_bad, num);
    serial_com1_puts("tessera: bad bytes: ");
    serial_com1_puts(num);
    serial_com1_puts("\n");

    if (payload_ready) {
        uint64_t entry = payload_entry;
        if (payload_check(TESS_INJECT_ADDR, m, &entry) == PAYLOAD_OK) {
            serial_com1_puts("tessera: running injected payload (unverified)\n");
            payload_run(entry);
        }
    }
    serial_root_run();
}
