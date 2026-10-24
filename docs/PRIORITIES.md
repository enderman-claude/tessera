# Priorities

Ranked by what unblocks the most work, then by effort. Effort is relative: S is a focused
session, M a few sessions, L a sustained push with several review rounds. "Needs" lists
what must be done first. "Decision" marks work that waits on the author.

## Ranked

| # | Task | Effort | Needs | Status |
|---|---|---|---|---|
| 0 | Review queue: merge PRs in order (tessera #3, #5 then #6, #4; oxinit #10, #11; tesseraOS #2) | S (author) | none | waiting on author |
| 1 | Physical page allocator | S | PR #3 | done in this branch |
| 2 | Paging and higher-half mapping | L | 1 | next |
| 3 | Timer IRQ and minimal scheduler | M | 2 (per-task stacks) | |
| 4 | Ring 3 and syscall entry | L | 2 | |
| 5 | Syscall ABI, finished | S | Decision | blocked |
| 6 | Capability table and Room/Town enforcement | L | 4 | |
| 7 | oxinit Linux leftovers (VM retest, fork-retry test, shutdown timing test) | S | PR #11 | |
| 8 | oxinit Tessera backend | L | 4, 5 | blocked |
| 9 | Shard: relocation test, entry contract | S-M | Decision on entry args | blocked |
| 10 | Ember Tessera target (ember #3) | M | 5 | blocked |
| 11 | teds, xteds, ember-gfx Tessera backends | L | 4, 6 | deferred |
| 12 | net (N1-N5) | L | 4 | deferred by author |

## Task breakdown

### 1. Physical page allocator (done here)
- 1.1 Unit `memory/pmm`: two bitmaps, one for not-usable pages (gaps and bad pages), one for in use. *Done.*
- 1.2 Host tests: init, exhaustion, free and reuse, double free, gap and bad-page rejection. *Done, 16 checks.*
- 1.3 Boot integration after the RAM test. *Done.*
- 1.4 QEMU marker with a numeric page count. *Done.*

### 2. Paging and higher-half mapping
- 2.1 Page table structures and a map function over the pmm
  - 2.1.1 Four-level table entries, flags, and walk-or-allocate
  - 2.1.2 Unit tests on fake tables in host memory
- 2.2 Identity map of kernel image and the pmm range, then switch CR3
  - 2.2.1 Confirm the kernel's own addresses after the switch
  - 2.2.2 QEMU marker for the switch
- 2.3 Higher-half mapping of the kernel
  - 2.3.1 Link address change and relocation check
  - 2.3.2 Handoff from the identity map
- 2.4 Guard pages below the kernel stack
  - 2.4.1 Unmap one page below the stack and test a fault

### 3. Timer IRQ and minimal scheduler
- 3.1 Remap PIC or enable APIC, and route the PIT to an IRQ
  - 3.1.1 Decide PIC or APIC (decision if the choice is not obvious)
  - 3.1.2 IRQ handler increments a tick counter, QEMU marker
- 3.2 Context switch from the timer handler
  - 3.2.1 Save and restore the register frame (`scheduler/context_switch` exists)
  - 3.2.2 Two kernel tasks alternate, QEMU marker
- 3.3 Per-task stacks from the pmm

### 4. Ring 3 and syscall entry
- 4.1 User code and data segments in the GDT, TSS stack
- 4.2 User mappings from task 2
- 4.3 `syscall`/`sysret` entry, number and argument convention from `abi.md`
- 4.4 One syscall (`exit`), QEMU marker from a user task

### 5. Syscall ABI, finished
- 5.1 Decision on spawn vs proc, signals, wait, blocking, shutdown (author)
- 5.2 Update `abi.md` and remove the open questions

### 6. Capability table and enforcement
- 6.1 Per-process handle table, close-on-exec flag
- 6.2 Check every syscall before side effects (`DENIED`, `NO_SUCH_CALL`)
- 6.3 Room and Town as manifest modes, audit log entries

### 7. oxinit Linux leftovers
- 7.1 Run `test/vm/test-vm.sh` on PR #11 (S)
- 7.2 Fork-retry test: needs a hook to force fork failure (M)
- 7.3 Shutdown timing test across two services with different grace periods (S)

## Decisions needed from the author
1. Merge order and approval for the open PRs (task 0).
2. Syscall ABI open questions (task 5): spawn vs proc, signals, wait, blocking, shutdown.
3. Shard entry contract: does entry take arguments?
4. Timer: PIC or APIC.
