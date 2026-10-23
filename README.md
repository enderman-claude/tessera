# tessera

An original x86_64 kernel that boots as a UEFI application (PE32+ `BOOTX64.EFI`).
No bootloader beyond the firmware. Built with plain `gcc` and `ld`, no mingw.

> **Status: early stage.** The image boots under QEMU with OVMF: it prints its boot stages,
> leaves boot services, runs the RAM test, and reaches the root console. It has not been
> tested on hardware. See [`docs/ROADMAP.md`](docs/ROADMAP.md).

## Documentation
- [Design](docs/DESIGN.md): goals, boot flow, source layout rules
- [Roadmap](docs/ROADMAP.md): what is done and what is next
- [Ecosystem](docs/ECOSYSTEM.md): which other projects are reused, and why
- [Journal](docs/JOURNAL.md): session-by-session log

## Source layout
Each unit is one directory, named after what it does:

```
src/<subsystem>/<unit>/<unit>.c        implementation
src/<subsystem>/<unit>/DEPS            optional: public headers the unit may include
include/tessera/<subsystem>/<unit>.h   the unit's only public interface
```

Current units:

| Subsystem | Unit | Purpose |
|---|---|---|
| boot | efi_entry | UEFI entry point, prints the boot stage and runs the boot path |
| console | serial_putc | write one byte to COM1, via a caller-supplied port writer |
| memory | bitmap_alloc | one bit per page allocator |
| scheduler | sort_priorities | insertion sort of task priorities |
| scheduler | pick_next | index of the highest-priority ready task |

## Rules
Enforced by `tools/check_layout.py`, which `make` runs before every build:
1. Each unit has exactly one `<unit>.c` and one public header.
2. A unit may include only its own header, standard headers, and headers in its `DEPS`.
3. Units reach each other only through public headers.
4. No source file is longer than 150 lines.

## Build
Requirements: `gcc`, `ld` (binutils with `i386pep` support), `python3`, `make`.

    make            # runs the layout check, then builds build/BOOTX64.EFI
    make clean

Build flags: `-std=c11 -Wall -Wextra -Werror -ffreestanding -fno-stack-protector -fno-pic -mno-red-zone -nostdlib`.

## Third-party code
External projects are submodules under `third_party/`, never copied in.
Currently: [facebook/zstd](https://github.com/facebook/zstd), planned for boot payload compression
(needs a freestanding shim, not yet written).

## Testing
`make test` runs the host tests. `make qemu-test` boots `build/BOOTX64.EFI` under QEMU with
OVMF (`qemu-system-x86_64` and the `ovmf` package) and checks the serial log for the boot
markers. The check passes when it sees the boot stages, the boot services exit, the RAM test
report with zero bad pages, and the root console prompt.

## Related repos

- [tesseraOS](https://github.com/enderman-claude/tesseraOS): the OS that composes this kernel with the other components.
- [shard](https://github.com/enderman-claude/shard): the binary format and loader for user programs. The kernel's SHARD loader is in the L2 layer.
- [tesseraOS-teds](https://github.com/enderman-claude/tesseraOS-teds): the display service. It runs as an unprivileged user service over this kernel's IPC endpoints and capabilities.

## License
Not yet chosen.
