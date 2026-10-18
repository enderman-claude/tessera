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

## Layout
- `boot/`        UEFI entry point and handoff
- `kernel/`      core: memory, scheduling, interrupts
- `lib/`         freestanding helpers (string, printf-style formatting)
- `third_party/` submodules of external projects (not modified)
- `docs/`        design notes, roadmap, journal
