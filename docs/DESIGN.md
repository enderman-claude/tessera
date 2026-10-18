# Tessera design

Tessera is an original x86_64 kernel that boots through UEFI.

## Goals
- Boot as a PE32+ EFI application, no bootloader beyond firmware.
- Keep the kernel core original. Reuse existing projects only for self-contained
  pieces (compression, hashing, parsing) via submodules under `third_party/`.
- Every component lives in its own directory and can be read on its own.

## Boot flow (planned)
1. Firmware loads `BOOTX64.EFI` (built from `boot/`).
2. `efi_main` locates the memory map and framebuffer via UEFI boot services.
3. Exit boot services, switch to the kernel's own page tables.
4. Hand control to `kernel/` with a boot info structure.

## Layers
Eight layers, L0 (firmware boundary) to L7 (applications). Access is granted by capabilities, not by ring alone.
See [spec/layers.md](spec/layers.md) and [spec/capabilities.md](spec/capabilities.md).

## Source layout (strict)
One directory per unit, named after what the unit does:

    src/<subsystem>/<unit>/<unit>.c       implementation
    src/<subsystem>/<unit>/DEPS           optional: public headers it may include
    include/tessera/<subsystem>/<unit>.h  the only public interface of a unit

Subsystems so far: boot, console, memory, scheduler. Units so far: efi_entry,
serial_putc, bitmap_alloc, sort_priorities, pick_next.

Rules, enforced by `tools/check_layout.py` (run by `make`):
1. Each unit has exactly one `<unit>.c` and one public header.
2. A unit includes only its own header, standard headers, and headers in DEPS.
3. Units never reach into each other's directories, only through public headers.
4. No source file exceeds 150 lines.

Third-party code is never placed here; it is a submodule under `third_party/`.
