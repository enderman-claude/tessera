# Decisions

Every open question from the specs and issues, decided. Each decision has a reason. A
decision can be overturned by changing its line here and the spec it points to. Merging is
not a decision made here: PRs are merged by the author after review.

## Processes (kernel.md)
- **P1. The kernel is PID 0.** Signals to PID 0 return DENIED for everyone, including root.
  *Reason: the kernel must not be stopped by any program, and an error is clearer than a
  silent ignore.*
- **P2. Other PIDs start at 1, per program view.** *Reason: same program, same numbers, on
  every run.*
- **P3. A program with no mode in its manifest runs in Room.** Town applies only when named.
  *Reason: Room is the default determinism sandbox; Town is strict and opt-in.*
- **P4. Town is a strict sandbox, not a security boundary.** After 3 refused calls, L4
  stops the program. *Reason: 3 is enough to catch a bug and not enough to be a
  denial-of-service lever.*
- **P5. A holiday process cannot ask its creator for more.** Such a request is refused and
  audited. A holiday process may return home and keeps nothing it was not granted.
  *Reason: no escalation path out of a holiday.*
- **P6. A program running as root stays in Room.** The root operator is unbounded, and
  determinism is claimed only against programs. *Reason: root programs keep the sandbox;
  the operator is the government.*
- **P7. stdin, stdout and stderr are granted at process creation in both modes.** Console
  writes are a sink and need no handle. *Reason: the console cannot change another program's
  results.*

## Filesystem and levels (isolation.md)
- **F1. A filesystem request names a directory.** The grant is read-only by default, and
  read-write needs an explicit flag. *Reason: narrow scope keeps results reproducible.*
- **F2. Every result carries its level (Pure or Filesystem).** *Reason: a reader must know
  which results are reproducible.*
- **F3. Production uses the live clock. Test runs use a fixed clock and a fixed seed by
  default.* *Reason: tests must reproduce; production needs real time.*
- **F4. Country records are readable by every process.** Programs are obfuscated by default
  and listed only by opt-in in the manifest. *Reason: visibility is a choice the program
  makes, and the default is the quieter one.*

## Permissions (permissions.md)
- **M1. godcivspy is the nine-bit rwxrwxrwx field.** It is stored and returned as a
  `uint16_t`. God is the owner, Civ the party, Spy the other.
  *Reason: the kernel's native name, from the author.*
- **M2. setuid, setgid and sticky are dropped.** Bits 9 to 15 are reserved and zero.
  *Reason: setuid conflicts with no ambient authority in Town.*
- **M3. godcivspy is the coarse identity layer, checked first.** Capabilities are the fine
  grants and are checked after it. *Reason: keeps the cheap check first and the spec's
  capability model intact.*
- **M4. The posture is three independent bits:** Room or Town, Pure or Filesystem, root or
  not root. *Reason: they are already independent in the specs.*
- **M5. Text and octal forms are produced by userland commands only.** The OS never produces
  them. *Reason: from the author.*

## Syscall ABI (abi.md)
- **A1. Calling convention:** the `syscall` instruction, SysV argument order with r10, return
  in rax as a signed value. *Reason: matches the existing kernel's signed-return style.*
- **A2. Errors are negative codes from the table in abi.md.** *Reason: already in use.*
- **A3. Handles are per-process, with a close-on-exec flag.** *Reason: matches the
  delegation rules in capabilities.md.*
- **A4. exec requires a spawn capability.** PID 1 (oxinit) receives spawn in its manifest.
  fork, wait and kill stay in the proc group. *Reason: spawn stays L4-scoped, and oxinit
  is the only program that starts others.*
- **A5. Signals use the Linux numbers.** *Reason: the least surprising choice, and oxinit
  already expects them.*
- **A6. wait takes a process handle and returns status in Linux wstatus encoding.**
  *Reason: oxinit's exit-code logic already decodes that encoding.*
- **A7. recv and wait block by default.** A nanosecond timeout argument, where 0 means block
  forever, is added to both when a caller needs it. *Reason: blocking is the simplest
  default; timeouts are opt-in.*
- **A8. Power off and reboot are in the kcall group and return STUB until a power driver
  exists.** *Reason: no driver exists yet, and STUB is honest about that.*
- **A9. time is in the clock group.** *Reason: F3 and P2 require it.*

## Layers and drivers
- **D1. The timer uses the PIC first, APIC later.** *Reason: the PIC is simpler, and the
  APIC is needed only for SMP.*

## oxinit (issues #1, #2)
- **O1. Socket timeouts stay constants:** 2 seconds for the server, 10 for oxictl.
  *Reason: moving them to !settings is a separate change, not needed now.*
- **O2. A service's leftover children get SIGKILL at once when its leader exits.**
  *Reason: this is the behaviour the PR implements; the author did not object.*
- **O3. The process-group pid-reuse window is accepted for now.** It gets a real fix in the
  reaper later. *Reason: the window is very small, and the fix is a larger change.*
- **O4. Power actions are root-only.** Policy exceptions come later. Shutdown runs as a
  fixed sequence, not a target, for now. *Reason: the simplest safe behaviour first.*

## Shard and ember
- **S1. The shard entry point takes argv and envp, like exec.** *Reason: oxinit needs a
  config handle, and argv is the existing mechanism.*
- **S2. Ember's Tessera target waits on the syscall ABI (A1 to A9).** *Reason: nothing
  can target a syscall interface that is not final.*

## Licence
- **L1. Each repo picks its own licence. The default is Apache-2.0, with a NOTICE file.**
  Apache-2.0 requires forks to keep the copyright and attribution notices, and the NOTICE
  file carries the project's own wording. The author wants a permissive licence that
  requires forks to reference the origin, and the NOTICE mechanism does that. A custom
  attribution clause beyond Apache-2.0 must be reviewed by a lawyer before it is used.
  *Reason: author rejected MIT for all repos. Apache-2.0 is established, permissive and
  already requires attribution in forks.*
- **L2. The joke licence is not adopted.** It is not a recognised licence and its legal
  effect is unclear. *Reason: a licence that can't be relied on is worse than a plain one.*
- **L3. oxinit's MIT PR (#13) is on hold** until the licence choice is confirmed.

## Merging
- **G1. The author merges.** I do not merge PRs. The merge order is in docs/PRIORITIES.md.
  *Reason: merging is a consequential action, and the author should make that call.*

## Deferred
- **N1. net (N1 to N5) stays deferred**, as the author asked.
