#include "tessera/boot/efi_entry.h"
#include "tessera/boot/efi_console.h"
#include "tessera/boot/efi_memmap_read.h"
#include "tessera/boot/efi_memmap_usable.h"
#include "tessera/boot/efi_exit_boot_services.h"
#include "tessera/lib/u64_to_dec.h"
#include "tessera/kernel/halt.h"
#include "tessera/boot/payload_check.h"
#include "tessera/boot/payload_run.h"

/* Raw payload injection is on by default. Build with TESS_RAW_PAYLOAD=0 to remove it. */
#ifndef TESS_RAW_PAYLOAD
#define TESS_RAW_PAYLOAD 1
#endif

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

#if TESS_RAW_PAYLOAD
    uint64_t entry = 0;
    int ps = payload_check(TESS_INJECT_ADDR, &map, &entry);
    efi_console_puts(st, "tessera: injected payload: ");
    efi_console_puts(st, payload_status_str(ps));
    efi_console_puts(st, "\n");
#endif

    efi_console_puts(st, "tessera: leaving boot services\n");
    s = efi_exit_boot_services(image, st, &map);
    if (s != EFI_SUCCESS) {
        efi_console_puts(st, "tessera: exit boot services failed\n");
        return s;
    }

#if TESS_RAW_PAYLOAD
    /* Re-check against the map that ExitBootServices accepted. The console is gone now. */
    if (ps == PAYLOAD_OK && payload_check(TESS_INJECT_ADDR, &map, &entry) == PAYLOAD_OK) {
        payload_run(entry);
    }
#endif

    kernel_halt();
}
