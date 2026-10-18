# Console

## Goal
Text output that works before exit boot services, through the UEFI console.

## Interface
`EFI_STATUS efi_console_puts(EFI_SYSTEM_TABLE *st, const char *s)`

- Converts ASCII to UTF-16 and calls `ConOut->OutputString`.
- `\n` is sent as `\r\n`.
- Strings longer than 255 characters are truncated at 255 characters.
- Non-ASCII bytes are not supported. They are sent as their low byte.

## Later
A raw serial writer (COM1, port 0x3F8) for after exit boot services. That is the
unit `console/serial_putc`, already present.
