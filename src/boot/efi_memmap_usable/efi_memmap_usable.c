#include "tessera/boot/efi_memmap_usable.h"

uint64_t efi_memmap_usable_pages(const struct efi_memmap *m)
{
    uint64_t pages = 0;
    if (m->desc_size == 0) {
        return 0;
    }
    UINTN count = m->used / m->desc_size;
    for (UINTN i = 0; i < count; i++) {
        const EFI_MEMORY_DESCRIPTOR *d =
            (const EFI_MEMORY_DESCRIPTOR *)(m->buf + i * m->desc_size);
        if (d->Type == EFI_CONVENTIONAL_MEMORY) {
            pages += d->NumberOfPages;
        }
    }
    return pages;
}
