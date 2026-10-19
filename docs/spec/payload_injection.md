# Raw payload injection

Lets the kernel run a payload without SHARD or ELF. The payload is placed in RAM by
something outside the kernel before the kernel starts. This is the "memory injection"
assumption: the kernel trusts that the bytes at the injection address were put there on
purpose.

## Who injects
- QEMU: `-device loader,file=payload.bin,addr=0x200000` with the descriptor prepended.
- A debugger writing memory before resuming.
- A boot script or test harness that writes into conventional RAM before entry.

The injector writes the descriptor and payload at `TESS_INJECT_ADDR` (default `0x200000`).
The kernel does not take the address from anything else.

## Descriptor
32 bytes, little-endian, followed immediately by the payload.

| Offset | Size | Field | Rule |
|---|---|---|---|
| 0 | 4 | magic | `TINJ` |
| 4 | 2 | version | 1 |
| 6 | 2 | reserved | 0 |
| 8 | 4 | size | 1 to 4 MiB, the payload bytes after the descriptor |
| 12 | 4 | entry | offset in the payload, less than `size` |
| 16 | 4 | checksum | sum of all payload bytes, mod 2^32 |
| 20 | 4 | flags | 0 |
| 24 | 8 | reserved | 0 |

## Checks the kernel makes
1. The descriptor lies in conventional RAM (checked before any byte is read).
2. Magic, version, flags, size, entry, and checksum are valid.
3. The descriptor and payload together lie inside one conventional RAM descriptor of the
   memory map in use.
4. The check runs twice: before ExitBootServices for the boot log, and again on the map
   ExitBootServices accepted. Only the second result decides whether to run.

## What the checks do not give
- No authorship or integrity beyond the checksum. The checksum catches corruption, not
  malice. Anyone who can write RAM before boot can pass these checks.
- No isolation. The payload runs in ring 0 with the kernel's privileges.
- No capabilities. Nothing from the layer model applies to it.

This is why the path is gated by `RAW_PAYLOAD` (default 1 in the Makefile; build with
`RAW_PAYLOAD=0` to remove it entirely) and why the boot log names it as an injected
payload. Production images should build with `RAW_PAYLOAD=0`.

## Memory
Before the payload runs, its pages must not be handed out by the allocator. The bitmap
allocator (`memory/bitmap_alloc`) does not yet know about this region. Until the kernel
allocator is wired to the memory map, the payload should be placed where the kernel does
not allocate, which the injector must arrange.

## Return
If the payload returns, the kernel halts. A payload that wants to keep running must not
return.
