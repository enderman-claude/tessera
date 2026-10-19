#ifndef TESSERA_MEMORY_RAM_TEST_H
#define TESSERA_MEMORY_RAM_TEST_H
#include <stddef.h>
#include <stdint.h>

/* Boot RAM test. Writes the pattern (0x42 by default) over a range, reads it back, and
 * marks every page with a bad byte in the bad-page list (see ram_bad_list.h). */

#define RAM_TEST_PATTERN 0x42u
#define RAM_TEST_RECORD_MAX 64u

struct ram_bad_byte {
    uint64_t addr;
    uint8_t wrote;
    uint8_t read;
};

struct ram_test_result {
    uint64_t pages_tested;
    uint64_t pages_bad;
    uint64_t bytes_bad;
    struct ram_bad_byte first[RAM_TEST_RECORD_MAX]; /* first bad bytes, for the report */
    uint32_t first_count;
};

/* Test-only hook, called once between the write pass and the read pass. Used to inject faults. */
typedef void (*ram_test_hook)(volatile uint8_t *base, uint64_t len, void *ctx);

/* addr and len must be page aligned and the range must be real RAM. */
int ram_test_range(uint64_t addr, uint64_t len, uint8_t pattern, ram_test_hook hook, void *ctx,
                   struct ram_test_result *out);

#endif
