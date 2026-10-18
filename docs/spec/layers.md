# Layers

tessera is split into three privilege domains. Code in one domain can only reach
another through a defined interface, and each interface is listed here.

| Layer | Privilege (x86_64) | Who runs there | Can touch hardware | Can call down into |
|---|---|---|---|---|
| Kernel space | ring 0 | boot, memory manager, scheduler, IPC, interrupt routing | yes, directly | nothing |
| Driver space | ring 3, with explicit I/O grants | device drivers as separate programs | only what the kernel grants | kernel space, through syscalls |
| User space | ring 3 | applications, services, tools | no | kernel space (syscalls), driver space (IPC) |

## Kernel space
Owns the machine. Everything here runs with full access.

Contents (planned):
- boot stub and boot info handoff (`boot/`, today stage 1)
- physical and virtual memory (`memory/`)
- scheduler and context switch (`scheduler/`)
- interrupt and exception routing, including delivery of device interrupts to drivers (`interrupt/`)
- syscall entry (`syscall/`)
- IPC between domains (`ipc/`)
- the SHARD loader, which places programs into memory for user and driver space (`loader/`)

Kernel space keeps only what must be privileged. Policy (which driver owns which
device, what a process may open) lives in user space as services.

## Driver space
Drivers run as separate programs, not inside the kernel. A driver crash should not
take down the machine.

- Each driver is a SHARD image loaded by the kernel loader.
- The kernel grants a driver: the MMIO ranges of its device, the I/O ports it needs,
  and the interrupt vectors it handles. Nothing else.
- A driver receives interrupts as messages from kernel space, not by running in an
  interrupt handler itself.
- Drivers talk to the rest of the system over IPC.
- A driver may need DMA. DMA is only allowed through a buffer the kernel allocates
  for it, with the device behind an IOMMU when one is present. Without an IOMMU, DMA
  is marked as a trusted driver and this is recorded in the driver's grant.

Open: whether to allow an in-kernel "trusted driver" path for performance (for
example, the timer or the serial console). Default answer: no, until there is a
measured need.

## User space
Applications, services, and tools. No direct hardware access.

- Talks to the kernel through a small syscall set (see below).
- Talks to drivers through IPC, through services that own the policy.
- Program format: SHARD, with the ELF subset for compatibility.

## Rules
1. Kernel space never calls into driver or user space. It only replies to them
   (syscalls, IPC, interrupt delivery).
2. Driver space never calls kernel internals. It only uses the syscalls and grants it is given.
3. User space never touches hardware. A request for hardware goes to a driver through IPC.
4. Every crossing between layers goes through a numbered interface listed in one
   header, so the boundary can be checked and tested.
5. A driver or user program that faults is stopped and reported, and the kernel keeps running.

## Interfaces (to be defined)
- `syscall/`: the syscall table. Each entry has a number, arguments, and a status.
- `ipc/`: message passing. A message has a sender, a receiver, a type, and a bounded payload.
- `driver/grant`: the list of hardware a driver may use, given by the kernel at load time.

## Mapping to the current code
Everything in `src/` today is kernel space, or pre-kernel boot code that runs under
UEFI. There is no driver or user space code yet. The next layer to build is the
kernel loader, followed by the syscall table and IPC.

## Build-order plan
1. Kernel loader: place a SHARD image in memory and start it at user privilege.
2. Syscall entry and a minimal syscall table (exit, write to console).
3. IPC between two user programs.
4. Driver space: load a driver with a grant, deliver an interrupt to it.
5. Services in user space that own device policy.
