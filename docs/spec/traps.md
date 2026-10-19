# Traps: GDT, IDT and exception entry

Layer L1 (kernel core). Encodings are host tested. The table loads, the stubs, and the
exception path have not run: they need the CPU and QEMU.

## GDT (`trap/gdt`)
- Three entries: null, 64-bit kernel code (selector 0x08, `0x00AF9A000000FFFF`), kernel data
  (selector 0x10, `0x00CF92000000FFFF`).
- `gdt_load()` runs `lgdt`, then reloads CS with a far return and sets DS, ES and SS.
- The kernel loads its own GDT and does not depend on the one UEFI left in place.

## IDT (`trap/idt`)
- 256 gates of 16 bytes. Interrupt gates, `type_attr` 0x8E (present, DPL 0, 64-bit interrupt gate).
- `idt_encode` splits the handler address into offset low, middle and high. IST is not used.
- `trap_init` installs gates 0..31 pointing at `isr_table`. Other vectors are not set yet.

## Exception entry (`trap/isr_table`, `trap/dispatch`)
- `isr_N` stubs for vectors 0..31. Vectors 8, 10, 11, 12, 13, 14, 17, 21, 29 and 30 push an error
  code from the CPU. Every other stub pushes a dummy 0. Each stub then pushes its vector number
  and jumps to `isr_common`.
- `isr_common` pushes the 15 general registers, aligns the stack to 16 bytes, calls
  `trap_dispatch(frame)`, restores, drops vector and error, and returns with `iretq`.
- `struct trap_frame` (in `isr_table.h`) is the layout in memory, lowest address first:
  r15 r14 r13 r12 r11 r10 r9 r8 rbp rdi rsi rdx rcx rbx rax, vector, error, rip, cs, rflags, rsp, ss.

## Dispatch
- `trap_dispatch` prints the vector, error code and rip on COM1, then calls the fatal hook.
- The kernel sets the hook to `kernel_panic`. L1 cannot include the L2 panic header, so the
  kernel passes it in (`trap_set_fatal`).
- If there is no hook, dispatch halts.
- Every exception is fatal for now. Recoverable faults are a later step.

## Not yet
- Timer interrupt (vector 32) and the scheduler tick: the next step.
- Page-fault handling (vector 14) that does more than panic. Needs paging.
- Interrupt controller setup (PIC remap or APIC). Only exceptions are installed.
- Running the exception path in QEMU, to check that the handler prints and reaches the Morse panic.
