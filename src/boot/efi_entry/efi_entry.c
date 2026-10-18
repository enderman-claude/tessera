#include "tessera/boot/efi_entry.h"

/* UEFI entry point. Stage 0: returns success so firmware can run the image. */
EFI_STATUS efi_entry(EFI_HANDLE image, EFI_SYSTEM_TABLE *system_table)
{
    (void)image;
    (void)system_table;
    return EFI_SUCCESS;
}
