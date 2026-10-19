#include "tessera/kernel/entry.h"
#include "tessera/kernel/halt.h"
#include "tessera/boot/ram_scan.h"
#include "tessera/boot/payload_check.h"
#include "tessera/boot/payload_run.h"
#include "tessera/memory/ram_test.h"
#include "tessera/memory/ram_bad_list.h"
#include "tessera/console/serial_com1.h"
#include "tessera/lib/u64_to_dec.h"
#include "tessera/syscall/debug_crash.h"

#define MAX_REGIONS 512

static struct ram_region regions[MAX_REGIONS];

void kernel_main(const struct efi_memmap *m, const uint64_t *keep, uint32_t n_keep,
                 int payload_ready, uint64_t payload_entry)
{
    char num[24];
    uint32_t nreg = ram_scan(m, keep, n_keep, regions, MAX_REGIONS);
    struct ram_test_result total = {0};
    ram_bad_reset();

    /* Fill every free RAM region with 0x42, read it back, and mark bad pages. */
    for (uint32_t r = 0; r < nreg; r++) {
        struct ram_test_result one;
        if (ram_test_range(regions[r].addr, regions[r].len, RAM_TEST_PATTERN, 0, 0, &one) == 0) {
            total.pages_tested += one.pages_tested;
            total.pages_bad += one.pages_bad;
            total.bytes_bad += one.bytes_bad;
        }
    }

    serial_com1_puts("tessera: ram test pattern 0x42 over ");
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

    /* Boot runs the crash test as root, so it is on by default. A non-root caller is
     * denied: debug_crash_authorized() is root or a loaded module, and nothing else. */
    struct principal boot_root = { .uid = 0, .is_module = 0 };
    serial_com1_puts("tessera: debug.crash test as root\n");
    (void)sys_debug_crash(&boot_root);

    if (payload_ready) {
        uint64_t entry = payload_entry;
        if (payload_check(TESS_INJECT_ADDR, m, &entry) == PAYLOAD_OK) {
            serial_com1_puts("tessera: running injected payload (unverified)\n");
            payload_run(entry);
        }
    }
    kernel_halt();
}
