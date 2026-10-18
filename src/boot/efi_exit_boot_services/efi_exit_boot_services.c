#include "tessera/boot/efi_exit_boot_services.h"

#define EXIT_ATTEMPTS 3

EFI_STATUS efi_exit_boot_services(EFI_HANDLE image, EFI_SYSTEM_TABLE *st, struct efi_memmap *m)
{
    EFI_STATUS s = EFI_INVALID_PARAMETER;
    for (int attempt = 0; attempt < EXIT_ATTEMPTS; attempt++) {
        s = efi_memmap_read(st, m);
        if (s != EFI_SUCCESS) {
            return s;
        }
        s = st->BootServices->ExitBootServices(image, m->map_key);
        if (s == EFI_SUCCESS) {
            return s;
        }
    }
    return s;
}
