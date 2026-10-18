#ifndef TESSERA_BOOT_EFI_CONSOLE_H
#define TESSERA_BOOT_EFI_CONSOLE_H
#include "tessera/boot/efi_types.h"

EFI_STATUS efi_console_puts(EFI_SYSTEM_TABLE *st, const char *s);

#endif
