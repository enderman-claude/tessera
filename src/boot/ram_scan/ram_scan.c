#include "tessera/boot/ram_scan.h"

static int contains(uint64_t lo, uint64_t len, uint64_t a)
{
    return a >= lo && a - lo < len;
}

uint32_t ram_scan(const struct efi_memmap *m, const uint64_t *keep, uint32_t n_keep,
                  struct ram_region *out, uint32_t max)
{
    uint32_t n = 0;
    if (m->desc_size == 0) return 0;
    uint64_t count = m->used / m->desc_size;
    for (uint64_t i = 0; i < count; i++) {
        const EFI_MEMORY_DESCRIPTOR *d =
            (const EFI_MEMORY_DESCRIPTOR *)(m->buf + i * m->desc_size);
        if (d->Type != EFI_CONVENTIONAL_MEMORY) continue;
        uint64_t len = d->NumberOfPages * 4096u;
        int in_use = 0;
        for (uint32_t k = 0; k < n_keep && !in_use; k++) {
            in_use = contains(d->PhysicalStart, len, keep[k]);
        }
        if (in_use) continue;
        if (n >= max) break;
        out[n].addr = d->PhysicalStart;
        out[n].len = len;
        n++;
    }
    return n;
}
