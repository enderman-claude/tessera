# Kernel

## kernel_halt
`__attribute__((noreturn)) void kernel_halt(void)`
Disables interrupts and halts forever (`cli; hlt`). Used as the last step of
stage 1 and as the response to any unrecoverable error once the kernel owns the
machine.

## Isolation
The kernel is a sandbox for determinism, not for security. It works like a room with
sound padding and no windows: nothing from outside is visible to a program, and no sound from
other programs gets in. The doors are always unlocked.

- **No windows.** A program can observe only what it was given. The kernel closes every
  other channel itself, so a program does not have to avoid them. That covers ambient time,
  the ids and state of other programs, and timing that depends on other programs. Where a
  program could see timing, the kernel pads it so it does not depend on other programs.
- **Doors always unlocked.** A program that asks for a resource is given it, with no prompt
  and no refusal. The kernel records the grant in the audit log, and the program's
  determinism level changes as described in `isolation.md`.
- **Not security.** The kernel does not need to resist a program that is trying to escape.
  Audit records make a dependency visible; they do not block it.

Stdin, stdout and stderr are granted at process creation. A program that holds only those
stays at the Pure level.
