#ifndef TESSERA_BOOT_RAM_SCAN_H
#define TESSERA_BOOT_RAM_SCAN_H
#include "tessera/boot/efi_memmap_read.h"

struct ram_region {
    uint64_t addr;
    uint64_t len;
};

/* Lists the conventional-memory descriptors of m that contain none of the keep addresses.
 * A descriptor that holds the kernel, its stack, or the payload is skipped whole, so the RAM
 * test never writes over anything in use. Returns the number of regions written to out. */
uint32_t ram_scan(const struct efi_memmap *m, const uint64_t *keep, uint32_t n_keep,
                  struct ram_region *out, uint32_t max);

#endif
