#include "tessera/boot/efi_console.h"

#define CONSOLE_MAX 255

EFI_STATUS efi_console_puts(EFI_SYSTEM_TABLE *st, const char *s)
{
    CHAR16 buf[CONSOLE_MAX + 2];
    size_t n = 0;
    while (*s && n < CONSOLE_MAX) {
        if (*s == '\n') {
            buf[n++] = 0x0D; /* CR before LF */
            if (n >= CONSOLE_MAX) break;
        }
        buf[n++] = (CHAR16)(unsigned char)*s++;
    }
    buf[n] = 0;
    return st->ConOut->OutputString(st->ConOut, buf);
}
