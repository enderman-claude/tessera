#ifndef TESSERA_BOOT_EFI_MEMMAP_READ_H
#define TESSERA_BOOT_EFI_MEMMAP_READ_H
#include "tessera/boot/efi_types.h"

/* Caller owns buf. used and map_key are set by efi_memmap_read. */
struct efi_memmap {
    uint8_t *buf;
    UINTN buf_size;
    UINTN used;
    UINTN map_key;
    UINTN desc_size;
    uint32_t desc_version;
};

EFI_STATUS efi_memmap_read(EFI_SYSTEM_TABLE *st, struct efi_memmap *m);

#endif
