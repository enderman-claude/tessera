# Boot RAM test

At boot, after ExitBootServices, the kernel fills every free RAM page with the pattern
`0x42`, reads it back, then does the same with `0xBD`. It marks any page with a wrong byte
in either pass as bad. Bad pages are never
handed out by the allocator.

## Where it runs
`kernel/entry` (`kernel_main`), called from the L0 boot stage through the L0→L1 handoff.
Nothing has been allocated yet at that point, so the test is the first user of free RAM.

## Which memory is tested
Only `EFI_CONVENTIONAL_MEMORY` descriptors from the memory map that ExitBootServices
accepted. Any descriptor that contains one of these addresses is skipped whole:
- the kernel image (address of `efi_entry`)
- the boot stack (address of a local in `efi_entry`)
- the injected payload (`TESS_INJECT_ADDR`), if any

Firmware memory (runtime services, ACPI, MMIO, reserved) is never conventional and is
never touched.

## Algorithm
1. Write `0x42` to every byte of each region.
2. Read every byte back. A byte that is not `0x42` is bad.
3. A page with any bad byte goes into the bad-page list (`memory/ram_bad_list`).
4. The first 64 bad bytes are recorded with address, written value, and read value.

Hook: a test-only callback runs between the write and read passes. It is used by host tests
to inject faults and is `NULL` in the kernel.

## Consumers
- `memory/ram_bad_apply` marks bad pages as used in a bitmap allocator bitmap. The
  allocator will call it before its first allocation.
- `ram_bad_contains(addr)` answers for a single address.

## Report
Over COM1 (serial, polled): pages tested, bad pages mapped, bad bytes. The UEFI console
is gone at this point, so serial is the only output.

## Limits
- Two patterns, `0x42` and `0xBD`. Together they catch a stuck bit at any position, since
  each bit is 1 in one of them. Faults that depend on the data (coupling between cells, or
  a bit that only fails under certain neighbours) are not covered.
- The test writes all free RAM. On a large machine it is slow, and it is destructive to
  anything the firmware left in conventional memory that the kernel does not know about.
- Only 4096 bad pages are recorded. Overflow is counted and reported.
- Scanning skips a whole descriptor when it contains a kept address, so the loss can be
  larger than the kept pages.
- Not run in firmware or QEMU yet.
