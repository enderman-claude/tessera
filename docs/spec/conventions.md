# Conventions

## Calling convention
- Code the firmware calls (the UEFI entry point and any protocol function it
  invokes) uses `EFIAPI`, which is `__attribute__((ms_abi))`.
- Internal kernel code uses the default SysV convention.

## Types
- UEFI types come from `tessera/boot/efi_types.h`. Nothing else redefines them.
- Kernel code uses `<stdint.h>` and `<stddef.h>` only.

## Errors
- Functions that can fail return `EFI_STATUS` (boot code) or `int` (kernel code,
  0 on success, negative on error).
- No function panics silently. Failures are reported on the console when it is
  available, then the kernel halts.

## Strings
- Kernel strings are NUL-terminated ASCII.
- UEFI text output takes UTF-16 (`CHAR16`). Conversion happens in
  `boot/efi_console` only.

## Build
- Freestanding: no libc, no hosted runtime.
- `-Werror`. A warning fails the build.
- Each unit is one `.c` file. `tools/check_layout.py` enforces the boundaries.
