#include "tessera/boot/efi_entry.h"
#include "tessera/boot/efi_console.h"
#include "tessera/boot/efi_memmap_read.h"
#include "tessera/boot/efi_memmap_usable.h"
#include "tessera/boot/efi_exit_boot_services.h"
#include "tessera/lib/u64_to_dec.h"
#include "tessera/kernel/entry.h"
#include "tessera/boot/payload_check.h"

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

    /* After exit, the console is gone. Hand over to the kernel, which tests RAM, runs the
     * payload (if one was accepted before exit), and halts. The keep list stops the RAM test
     * from writing over this image, its stack, or the injected payload. */
    uint64_t keep[3] = {
        (uint64_t)(uintptr_t)&efi_entry,
        (uint64_t)(uintptr_t)&s,
        (uint64_t)TESS_INJECT_ADDR,
    };
#if TESS_RAW_PAYLOAD
    kernel_main(&map, keep, 3, ps == PAYLOAD_OK, entry);
#else
    kernel_main(&map, keep, 3, 0, 0);
#endif
}
