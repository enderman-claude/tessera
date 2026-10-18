# Layers

tessera has eight layers, numbered from most to least privileged. A layer may only
call the layer directly below it or a layer that explicitly exports an interface to it.
Calls upward are replies, never requests (interrupts and IPC replies excepted; see rules).

| # | Layer | Privilege | Lifetime | Holds |
|---|---|---|---|---|
| L0 | Firmware boundary | ring 0, under UEFI | until ExitBootServices | boot stage 1 (`boot/`) |
| L1 | Kernel core | ring 0 | always | page tables, context switch, trap and syscall entry, timer tick |
| L2 | Kernel services | ring 0 | always | capability table, IPC, SHARD loader, syscall dispatch, interrupt routing |
| L3 | System services | ring 3, trusted | always | device manager, supervisor (starts everything), resource broker |
| L4 | Mediated drivers (D0) | ring 3 | while device is bound | no hardware capabilities; every hardware action is a validated kernel call |
| L5 | Direct drivers (D1, D2) | ring 3 | while device is bound | BAR, MSI and optional DMA for one device; bus controllers also enumerate children |
| L6 | User services | ring 3, unprivileged | always | filesystem, network, display, etc.; reach hardware only through L4/L5 IPC |
| L7 | Applications | ring 3, sandboxed | per process | a minimal capability set chosen by the supervisor |

## What each layer may do

**L0 Firmware boundary.** Runs once, before the kernel exists. Ends at ExitBootServices.
Nothing from L0 survives into later layers except the memory map it hands over.

**L1 Kernel core.** Mechanism only: map and unmap pages, switch contexts, enter and leave
traps, count ticks. It contains no policy: no scheduling decisions beyond round robin,
no naming, no device knowledge. Its size is a budget and is reviewed when it grows.

**L2 Kernel services.** Policy-free plumbing that all higher layers need: the capability
table, the IPC mechanism, the loader that places a SHARD image and installs its capabilities,
syscall dispatch that checks capabilities before doing anything, and routing of
interrupts to the driver that holds the matching IRQ capability.

**L3 System services.** Trusted, but still ring 3. The device manager enumerates the bus
(PCI) and mints device capabilities for drivers. The supervisor decides which program
gets which capabilities at start. The resource broker hands out memory and CPU budgets.
L3 can crash and be restarted; the kernel does not depend on it staying up except for
device binding.

**L4 Mediated drivers (class D0).** Hold no bus capabilities. Timers, interrupt controller,
serial, PCI configuration reads and allowlisted writes, power, platform devices. Each
hardware action is a kernel call that the kernel checks. Slowest, and the default class.

**L5 Direct drivers (classes D1 and D2).** Own one device function. They may use the BAR
mappings, MSI vectors, and DMA window they were granted, and write their own device's
data-path registers directly. They may not write config space, change BAR placement,
use port I/O, or touch other devices. A bus controller (D2) may also enumerate and
delegate its children. See [driver_classes.md](driver_classes.md).

**L6 User services.** No hardware capabilities at all. A service asks a driver to do
hardware work through IPC. The supervisor grants a service the endpoints it needs and
nothing else.

**L7 Applications.** Sandboxed. Get memory, their own IPC endpoints, and what the
supervisor names. No driver endpoints unless the supervisor grants them.

## Rules
1. A layer calls only downward, or through an IPC endpoint or syscall it was granted.
2. Every syscall is checked against the caller's capability table. No capability,
   no access. The check is in L2, not in the caller.
3. Capabilities are only created by L2 at load time, from a manifest supplied by L3.
   A program cannot create one.
4. A capability can be delegated only in a weaker form (narrower range, fewer rights).
5. A faulting program at any layer is stopped and reported. A faulting driver or service
   is restarted by L3; the kernel keeps running.
6. DMA is never available without an IOMMU domain in the default policy.
7. Access class (D0, D1, D2) is part of the manifest, checked by the loader against the capabilities requested.
8. Layer membership is part of the manifest, not of the SHARD image. A SHARD image
   cannot claim a higher layer for itself.

## Source layout mapping
Each layer maps to a source subsystem. A unit's layer is its subsystem's layer.
- `boot/` → L0
- `memory/`, `scheduler/` (mechanism), `trap/`, `timer/` → L1
- `cap/`, `ipc/`, `loader/`, `syscall/`, `irq/` → L2
- `svc/device_mgr/`, `svc/supervisor/`, `svc/broker/` → L3
- `driver/mediated/` → L4 (D0), `driver/direct/` → L5 (D1, D2), chosen by manifest
- user programs live outside the kernel tree.

The layout checker will enforce that L1 never includes L2+ headers. This is not yet in
`tools/check_layout.py`.

## Build order
1. L0 → L1 handoff: enter the kernel core with the memory map.
2. L1: paging, traps, timer.
3. L2: capability table, loader (places one SHARD image, installs its caps), syscall dispatch with the console write call.
4. L3: supervisor starts one user program at L7 and reports its exit.
5. L4: serial driver (D0) through kernel calls, via IPC.
6. L5: a D1 driver for a device with MSI but no DMA, then DMA only after the IOMMU path is tested on hardware.
