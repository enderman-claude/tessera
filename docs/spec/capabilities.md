# Capabilities

Every resource a program can reach is named by a capability. A program starts with
the capabilities its manifest lists, and nothing else. There is no ambient authority:
no path, PID, or global name grants access.

## Kinds

| Kind | Grants | Layers that may hold it |
|---|---|---|
| `mem` | a virtual range, rw or rx | L1-L7 (as manifest allows) |
| `mmio` | a physical range from a device BAR, rw | L4, L5 |
| `irq` | one interrupt vector, delivered as a message | L4, L5 |
| `port` | a range of x86 I/O ports, enforced by the TSS I/O bitmap | L4 only |
| `dma` | a DMA window inside one IOMMU domain | L5 only |
| `endpoint` | send or receive on one IPC endpoint | L3-L7 |
| `syscall-group` | a named group of syscalls (see below) | L3-L7 |
| `spawn` | the right to start a program at a layer below the caller's | L3 only |

## Syscall groups
- `core`: exit, yield, time. Everyone.
- `mem`: map and unmap own pages. L1-L7.
- `ipc`: send, receive, reply on held endpoints. L3-L7.
- `console`: write to the boot console. L3 and L6 by default.
- `dev`: bind and unbind a device, claim IRQs and MMIO. L3 only, and used by drivers only through L3 grants.
- `spawn`: start a program. L3 only.

Groups not in a program's manifest are absent from its syscall table. Calls to them
fail with "no such syscall", not "denied", so a program cannot probe what it lacks.

## Delegation
A holder of a capability may give a copy to another program only if the copy is weaker:
- a sub-range of a memory or MMIO range
- read in place of read-write
- a send-only endpoint in place of send-and-receive

Delegation never creates a capability the delegator does not hold.

## Revocation
Every capability records the one it was derived from. Revoking a capability revokes
all its derivatives. L3 revokes a driver's capabilities when the device is unbound
or the driver is restarted.

## Enforcement point
L2 checks every syscall and every IPC send against the caller's table. The check happens
before any side effect. A failed check returns an error and is written to the audit log
(kind, caller, attempted object). It does not stop the program unless the program
repeats the violation past a limit set by L3.

## Not yet decided
- Whether capabilities are per-process tables or a global object space with handles.
- How manifests are signed and verified. Until then, the supervisor is trusted to write them.
