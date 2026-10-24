# Kernel

## kernel_halt
`__attribute__((noreturn)) void kernel_halt(void)`
Disables interrupts and halts forever (`cli; hlt`). Used as the last step of
stage 1 and as the response to any unrecoverable error once the kernel owns the
machine.

## Sandbox modes
The kernel runs each program in one of two sandboxes, chosen in its manifest. The
`room` mode is described first, then `town`.

### Room (determinism)
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

### Town (strict)
A strict sandbox that allows everything a program should be able to do, and not much more.

- **Exactly the grants it needs.** Every handle, syscall group and resource is named in the
  manifest. Nothing outside the manifest is allowed, and nothing is granted on request.
- **Checked on every call.** Each syscall is checked against the manifest before any side
  effect. A call outside the manifest is `DENIED`, or `NO_SUCH_CALL` if the group is absent.
- **No automatic grants.** A filesystem request from a Town program is refused unless the
  manifest already grants that scope. Requests are recorded whether granted or refused.
- **Audited.** Every refusal is written to the audit log with the caller and the attempted object.

### Root: the government
Root is the government. Nothing can be hidden from it.

- **Root sees everything.** Root can observe every town, the inside of every town, every
  program, obfuscated or not, and every record. Town borders and obfuscation do not apply to root.
- **Root can read memory.** Root can read the memory of any process, in any town, with no
  handle needed. Memory reads are observations like any other, and are audited the same way.
- **Every root observation is audited.** Root has no hidden access: each observation is
  written to the audit log with the observed object, so the government's view is itself visible.
- **Root sees, and this spec does not say more yet.** Whether root may also act inside a
  town (grant, revoke, stop a program) is not decided here. Visibility is settled; power is not.

### Country records
The country records are the one public window. They list every town and every program
that is not obfuscated. They are the only way a process learns that another town exists.

- **Read-only.** A process can read the records. It cannot change them, and no process
  can change them except the kernel, when a town is created or a program is listed.
- **Listed or obfuscated.** A program is listed in the records only if its manifest says so.
  An obfuscated program does not appear in the records, and its town shows only as a
  town, with no program names.
- **Reading reveals no state.** The records show names and existence, not the contents of
  a town or the state of a program. Reading them does not let a process observe anything
  inside another town.
- **Fixed for a run.** The records a process reads are a snapshot taken when the process
  starts, so the same program sees the same records on every run. This keeps results
  deterministic.
- **Audited.** Each read is recorded in the audit log.

Reading the records does not count as crossing a town border.

### Towns and holidays
- **A town is an observation boundary.** Processes in different towns cannot see each other:
  no pids, no state, no timing, no handles across the boundary. A process in one town cannot
  peek at a process in another.
- **A process can take a holiday.** A process may create a new town and move itself into it,
  with a set of grants it chooses. It controls that town: it defines what the town contains
  and can see into it.
- **A holiday town is no wider than its creator.** Grants given to a holiday town must be a
  subset of what the creator holds, by the delegation rules in `capabilities.md`. A holiday
  can never grant more than the creator has.
- **Crossing the border is by handle only.** Anything passed in or out of a town is an
  explicit handle, such as an endpoint, and is audited like any other grant.

Town is the mode for programs whose behaviour must be bounded, and it is the one that a
security argument can rely on. Room, by contrast, gives programs freely granted resources
for reproducibility.

Stdin, stdout and stderr are granted at process creation in both modes.
