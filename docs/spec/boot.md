# Boot

## Goal
Run as a UEFI application, report what the firmware gave us, take ownership of
the machine, and halt until the kernel has more to do.

## Stage 1 (current target)
1. Firmware calls `efi_entry(image, system_table)`.
2. Print `tessera: boot stage 1`.
3. Read the UEFI memory map into a stack buffer.
   Failure: print an error line and return the status.
4. Count usable pages (`EFI_CONVENTIONAL_MEMORY`) and print the count.
5. Exit boot services. The memory map is re-read inside this step so the map key
   is current. Retry up to 3 times on `EFI_INVALID_PARAMETER`.
6. Halt with interrupts disabled.

## Stage 1 limits
- Memory map buffer is 16 KiB. A map larger than that fails with
  `EFI_BUFFER_TOO_SMALL` and is reported, not silently truncated.
- The console is unavailable after step 5. Nothing prints after that point.

## Not in stage 1
- Loading a kernel image or payload.
- Paging, GDT, IDT.
- Framebuffer (GOP).

## Build output
`build/BOOTX64.EFI`, a PE32+ image with `DYNAMIC_BASE` set and a `.reloc` section.
Position-independent code (`-fpic`) is required, because the firmware may load
the image at any address.
