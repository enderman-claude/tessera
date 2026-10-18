#include "tessera/boot/efi_entry.h"
#include "tessera/boot/efi_console.h"
#include "tessera/boot/efi_memmap_read.h"
#include "tessera/boot/efi_memmap_usable.h"
#include "tessera/boot/efi_exit_boot_services.h"
#include "tessera/lib/u64_to_dec.h"
#include "tessera/kernel/halt.h"

#define MEMMAP_BYTES 16384

EFI_STATUS EFIAPI efi_entry(EFI_HANDLE image, EFI_SYSTEM_TABLE *st)
{
    uint8_t buf[MEMMAP_BYTES];
    struct efi_memmap map = { .buf = buf, .buf_size = sizeof buf };
    char num[24];
    EFI_STATUS s;

    efi_console_puts(st, "tessera: boot stage 1\n");

    s = efi_memmap_read(st, &map);
    if (s != EFI_SUCCESS) {
        efi_console_puts(st, "tessera: memory map read failed\n");
        return s;
    }

    u64_to_dec(efi_memmap_usable_pages(&map), num);
    efi_console_puts(st, "tessera: usable pages: ");
    efi_console_puts(st, num);
    efi_console_puts(st, "\n");

    efi_console_puts(st, "tessera: leaving boot services\n");
    s = efi_exit_boot_services(image, st, &map);
    if (s != EFI_SUCCESS) {
        efi_console_puts(st, "tessera: exit boot services failed\n");
        return s;
    }

    kernel_halt();
}
