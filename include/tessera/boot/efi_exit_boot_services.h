#ifndef TESSERA_BOOT_EFI_EXIT_BOOT_SERVICES_H
#define TESSERA_BOOT_EFI_EXIT_BOOT_SERVICES_H
#include "tessera/boot/efi_memmap_read.h"

/* Re-reads the memory map into m and exits boot services. Retries up to 3 times. */
EFI_STATUS efi_exit_boot_services(EFI_HANDLE image, EFI_SYSTEM_TABLE *st, struct efi_memmap *m);

#endif
