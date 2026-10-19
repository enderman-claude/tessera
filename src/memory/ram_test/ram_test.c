#include "tessera/memory/ram_test.h"
#include "tessera/memory/ram_bad_list.h"

int ram_test_range(uint64_t addr, uint64_t len, uint8_t pattern, ram_test_hook hook, void *ctx,
                   struct ram_test_result *out)
{
    if ((addr & (RAM_PAGE - 1)) || (len & (RAM_PAGE - 1)) || len == 0) {
        return -1;
    }
    volatile uint8_t *base = (volatile uint8_t *)(uintptr_t)addr;

    for (uint64_t i = 0; i < len; i++) {
        base[i] = pattern;
    }
    if (hook) {
        hook(base, len, ctx);
    }

    out->pages_tested = len / RAM_PAGE;
    out->pages_bad = 0;
    out->bytes_bad = 0;
    out->first_count = 0;

    for (uint64_t p = 0; p < out->pages_tested; p++) {
        int page_bad = 0;
        for (uint64_t i = p * RAM_PAGE; i < (p + 1) * RAM_PAGE; i++) {
            uint8_t got = base[i];
            if (got != pattern) {
                out->bytes_bad++;
                if (!page_bad) {
                    out->pages_bad++;
                    ram_bad_add(addr + p * RAM_PAGE);
                    page_bad = 1;
                }
                if (out->first_count < RAM_TEST_RECORD_MAX) {
                    struct ram_bad_byte *b = &out->first[out->first_count++];
                    b->addr = addr + i;
                    b->wrote = pattern;
                    b->read = got;
                }
            }
        }
    }
    return 0;
}
