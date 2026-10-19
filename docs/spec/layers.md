# Layers

tessera has nine layers, numbered from most to least privileged. A layer may only call
the layer directly below it, or a layer that explicitly exports an interface to it.
Calls upward are replies, never requests (interrupts and IPC replies excepted; see rules).

| # | Layer | Privilege | Lifetime | Holds |
|---|---|---|---|---|
| L0 | Firmware boundary | ring 0, under UEFI | until ExitBootServices | boot stage 1 (`boot/`) |
| L1 | Kernel core | ring 0 | always | page tables, context switch, trap and syscall entry, timer tick |
| L2 | Kernel services | ring 0, built-in | always | capability table, IPC, SHARD loader, syscall dispatch, interrupt routing, module loader |
| L3 | Kernel modules | ring 0, loadable | from boot until shutdown | optional kernel extensions that register hooks into L2 (see [kernel_modules.md](kernel_modules.md)) |
| L4 | System services | ring 3, trusted | always | device manager, supervisor (starts everything), resource broker |
| L5 | Mediated drivers (D0) | ring 3 | while device is bound | no hardware capabilities; every hardware action is a validated kernel call |
| L6 | Direct drivers (D1, D2) | ring 3 | while device is bound | BAR, MSI and optional DMA for one device; bus controllers also enumerate children |
| L7 | User services | ring 3, unprivileged | always | filesystem, network, display, etc.; reach hardware only through L5/L6 IPC |
| L8 | Applications | ring 3, sandboxed | per process | a minimal capability set chosen by the supervisor |

## What each layer may do

**L0 Firmware boundary.** Runs once, before the kernel exists. Ends at ExitBootServices.
Nothing from L0 survives into later layers except the memory map it hands over.

**L1 Kernel core.** Mechanism only: map and unmap pages, switch contexts, enter and leave
traps, count ticks. No policy: no scheduling decisions beyond round robin, no naming, no
device knowledge. Its size is a budget and is reviewed when it grows.

**L2 Kernel services.** Policy-free plumbing that all higher layers need: the capability
table, the IPC mechanism, the loader that places a SHARD image and installs its
capabilities, syscall dispatch that checks capabilities first, interrupt routing to the
driver holding the matching IRQ capability, and the module loader that starts L3 modules.

**L3 Kernel modules.** Ring 0 code that assists the kernel. A module is not a driver and
not a service. Built-in basics live in L2; modules add more, loaded during boot. Modules
are trusted code: they run with kernel privilege, so the module list is part of the
trusted computing base. Full rules are in [kernel_modules.md](kernel_modules.md).

**L4 System services.** Trusted, but ring 3. The device manager enumerates the bus and
mints device capabilities for drivers. The supervisor decides which program gets which
capabilities at start. The resource broker hands out memory and CPU budgets. L4 can crash
and be restarted; the kernel depends on it only for device binding.

**L5 Mediated drivers (class D0).** Hold no bus capabilities. Timers, interrupt controller,
serial, PCI configuration reads and allowlisted writes, power, platform devices. Each
hardware action is a kernel call that the kernel checks. Slowest, and the default class.

**L6 Direct drivers (classes D1 and D2).** Own one device function. They may use the BAR
mappings, MSI vectors, and DMA window they were granted, and write their own device's
data-path registers directly. They may not write config space, change BAR placement,
use port I/O, or touch other devices. A bus controller (D2) may also enumerate and
delegate its children. See [driver_classes.md](driver_classes.md).

**L7 User services.** No hardware capabilities. A service asks a driver to do hardware
work through IPC. The supervisor grants a service the endpoints it needs and nothing else.

**L8 Applications.** Sandboxed. Get memory, their own IPC endpoints, and what the
supervisor names. No driver endpoints unless the supervisor grants them.

## Rules
1. A layer calls only downward, or through an IPC endpoint or syscall it was granted.
2. Every syscall is checked against the caller's capability table. No capability, no
   access. The check is in L2, not in the caller.
3. Capabilities are only created by L2 at load time, from a manifest supplied by L4.
   A program cannot create one.
4. A capability can be delegated only in a weaker form (narrower range, fewer rights).
5. A faulting program at L4 or above is stopped and reported. A faulting driver or service
   is restarted by L4; the kernel keeps running. A faulting L3 module is a kernel fault:
   the module is disabled if it is optional, and boot halts if it is required.
6. DMA is never available without an IOMMU domain in the default policy.
7. Access class (D0, D1, D2) is part of the manifest, checked by the loader against the
   capabilities requested.
8. Layer membership is part of the manifest, not of the SHARD image. A SHARD image cannot
   claim a higher layer for itself.

## Handoff
L0 enters L1 once, at a single header (`kernel/halt.h` today; the kernel entry point later).
This is the only upward include the layout checker allows. Any other include of a higher
layer is an error.

## Source layout mapping
- `boot/` -> L0
- `memory/`, `scheduler/` (mechanism), `trap/`, `timer/` -> L1
- `cap/`, `ipc/`, `loader/`, `syscall/`, `irq/`, `modload/` -> L2
- `module/` -> L3 (built-in modules are linked into the kernel; loadable ones are SHARD images)
- `svc/device_mgr/`, `svc/supervisor/`, `svc/broker/` -> L4
- `driver/mediated/` -> L5 (D0), `driver/direct/` -> L6 (D1, D2), chosen by manifest
- user programs live outside the kernel tree.

The layout checker enforces that a unit includes only headers from its own or a more
privileged layer (lower number), with the L0 to L1 handoff as the one exception. The
subsystem-to-layer table is in `tools/check_layout.py` (`LAYER`).

## Build order
1. L0 -> L1 handoff: enter the kernel core with the memory map.
2. L1: paging, traps, timer.
3. L2: capability table, loader, syscall dispatch with console write.
4. L3: built-in module init order, then one loadable module from boot media.
5. L4: supervisor starts one L8 program and reports its exit.
6. L5: serial driver (D0) through kernel calls, via IPC.
7. L6: a D1 driver with BAR and MSI only, then DMA after the IOMMU path works on hardware.
