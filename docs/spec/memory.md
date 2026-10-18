# Memory

Draft, not yet coded.

## Physical pages
- Page size is 4096 bytes.
- The allocator tracks pages with one bit each (`memory/bitmap_alloc`).
- Initial state: all pages used. Pages marked `EFI_CONVENTIONAL_MEMORY` in the
  firmware memory map are freed. Everything else stays used.
- The bitmap lives in memory from the map, taken before exit boot services.

## Not yet defined
- Virtual memory and paging (planned after stage 1).
- Kernel heap.
