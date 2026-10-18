#ifndef TESSERA_BOOT_EFI_MEMMAP_USABLE_H
#define TESSERA_BOOT_EFI_MEMMAP_USABLE_H
#include "tessera/boot/efi_memmap_read.h"

/* Sum of NumberOfPages for descriptors of type EFI_CONVENTIONAL_MEMORY. */
uint64_t efi_memmap_usable_pages(const struct efi_memmap *m);

#endif
