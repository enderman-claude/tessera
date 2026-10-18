#ifndef TESSERA_BOOT_EFI_ENTRY_H
#define TESSERA_BOOT_EFI_ENTRY_H
#include "tessera/boot/efi_types.h"

EFI_STATUS efi_entry(EFI_HANDLE image, EFI_SYSTEM_TABLE *system_table);

#endif
