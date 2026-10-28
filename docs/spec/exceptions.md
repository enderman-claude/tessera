# Exceptions and faults: plan

Status: plan, not implemented. Decisions marked **Open** need an answer before building.
Related: `docs/spec/traps.md` (current behaviour: every exception is fatal), `docs/spec/capabilities.md`
(enforcement point), master plan enderman-claude/tesseraOS#1, tessera#1 (milestones M1 to M3).

## 1. Principles

1. **Kernel faults panic; user faults kill the process.** A fault in ring 0 is a kernel bug and
   goes to the Morse panic. A fault in ring 3 is a program bug. It ends that program and is
   reported. It never panics the kernel.
2. **Every termination has a reason.** The kernel records one reason per process exit: a kind,
   a code, the faulting address (if any), the instruction pointer, and the process id. The
   reason goes to the audit log and to the parent (L4 supervisor, then oxinit).
3. **Invalid input is refused before it runs.** Bad binaries fail at load time, and the spawn
   call returns an error. No partial image is ever executed.
4. **Invalid requests return errors; they do not fault.** A bad system call argument returns an
   error code. It only ends the process if the program repeats the violation past the limit set
   by L4 (`capabilities.md`).
5. **No retries for invalid code.** A fault that cannot be fixed by the kernel (bad opcode, bad
   address with no mapping, protection violation) is final. Only demand paging and the timer
   are resumable, and only when the paging milestone adds them.
6. **Deterministic reporting.** The same binary with the same input produces the same reason.
   Tests check the exact reason, not just "it died".

## 2. Taxonomy

Every invalid behaviour belongs to exactly one kind. The kind decides the detection point, the
kernel action, and the reason reported.

| Kind | Examples | Detected by | Ring 3 action | Reason code |
|---|---|---|---|---|
| `ill-insn` | invalid opcode, UD2, unsupported instruction (e.g. AVX when disabled) | CPU, vector 6 | kill | `ILL` |
| `div-zero` | integer divide by zero, quotient overflow | CPU, vector 0 | kill | `FPE` |
| `priv-insn` | HLT, CLI, LGDT, MOV to CR, IO port without grant | CPU, vector 13 | kill | `PRIV` |
| `bad-address` | non-canonical address, unmapped address (incl. null) | CPU, vector 14 or 13 | kill | `SEGV` |
| `protection` | write to read-only, user access to kernel page | CPU, vector 14 | kill | `SEGV` |
| `exec-data` | instruction fetch from a non-executable page (NX) | CPU, vector 14 | kill | `SEGV` |
| `stack-overflow` | access to the guard page below a stack | CPU, vector 14 | kill | `STACK` |
| `misaligned` | unaligned access, only if alignment checking is enabled | CPU, vector 17 | kill | `BUS` |
| `fpu-unavail` | x87 or SSE used before the FPU state is set up | CPU, vector 7 or 16/19 | kill (until FPU is enabled) | `ILL` |
| `breakpoint` | INT3 left in a program | CPU, vector 3 | kill | `TRAP` |
| `bad-syscall` | unknown number, wrong argument count | kernel | error return | (none, see §4) |
| `bad-pointer` | syscall argument points outside the caller | kernel copy | error `EFAULT` | (none) |
| `denied` | call in a group not in the manifest, capability missing | kernel | error (absent group: "no such syscall") | (none, see §4) |
| `bad-handle` | closed or foreign handle | kernel | error `EBADF`-style | (none) |
| `bad-image` | invalid SHARD or ELF image at spawn | loader | spawn fails | (none, see §3) |
| `oom` | allocation fails inside the program's runtime | runtime or kernel | program panic (`code 3`) or error | `PANIC` |
| `program-panic` | Ember panics: index out of bounds, divide by zero in Ember code, invalid array handle | Ember runtime | exit | `PANIC` with the Ember panic code |
| `timeout` | program never yields (CPU budget exceeded) | scheduler | kill | `TIMEOUT` (**Open**, §7) |
| `kernel-fault` | any fault in ring 0, invariant violation | CPU or kernel check | panic (Morse) | (kernel panic, not a process reason) |

`bad-syscall`, `bad-pointer`, `denied` and `bad-handle` are reported as error returns, not as
faults, so a program can handle them. The exception is the repeat limit in §4.

## 3. Bad binaries (load time)

The loader must reject an image before anything in it runs. The error codes already exist in
shard (`shard_open`, `elf_load`); this plan maps each one to a spawn result.

| Loader error | Meaning | Spawn result |
|---|---|---|
| `SHARD_ERR_TOO_SHORT` / `ELF_ERR_RANGE` | header or table runs past the file | `EBADIMG` |
| `SHARD_ERR_MAGIC` | not a SHARD image | `EBADIMG` (then tried as ELF, if ELF is allowed) |
| `SHARD_ERR_VERSION`, `SHARD_ERR_MACHINE` | unknown version or not x86-64 | `EBADIMG` |
| `SHARD_ERR_RANGE`, `SHARD_ERR_SECTION` | section or segment outside the image, or overlapping | `EBADIMG` |
| `SHARD_ERR_RELOC`, `ELF_ERR_RELOC` | unknown relocation type, or undefined symbol | `EBADIMG` |
| `SHARD_ERR_DEST`, `ELF_ERR_DEST` | image does not fit the destination | `ENOMEM` |
| `SHARD_ERR_UNSUPPORTED` | feature not in v1 (flags, dynamic linking stubs) | `ENOTSUP` |
| `ELF_ERR_NEEDED` | depends on a shared library | `ENOTSUP` |
| `ELF_ERR_SEGMENT` | bad program header | `EBADIMG` |

Additional checks the loader must make (**Open**: confirm each is in the v1 spec):
- The entry point lies inside an executable section. Today `shard_open` checks only that it is
  less than `image_size`.
- No section is both writable and executable (W^X). This is a policy choice; see §7.
- The image contains no more sections or relocations than the header counts allow (already checked).
- Every image is loaded into fresh pages. A failed load leaves no partly mapped memory behind.

A spawn that fails returns the error to the parent. A failed load never produces a process, so
there is no exit reason to report.

## 4. System call violations

- **Unknown or unpermitted call.** Calls not in the program's manifest are absent from its syscall
  table (`capabilities.md`), so they return "no such syscall". Calls in a group the program
  holds, but with a bad argument, return an error code.
- **Repeat limit.** Each process has a count of recorded violations. When it reaches the limit
  set by L4, the process is killed with reason `DENIED`. The limit is per supervisor, from the
  manifest. **Open:** the default limit, and whether it is per process or per time window.
- **Pointer arguments.** The kernel validates every user pointer against the caller's mappings
  before use (`copy_from_user` style). A bad pointer returns `EFAULT`. The kernel must never
  fault on a user address during a syscall. This needs a fixup table for the few kernel
  instructions that touch user memory (**Open:** table or per-call checks).

## 5. Reporting the reason

The kernel creates one record per exit:

    struct exit_reason {
        uint32_t kind;        /* taxonomy kind, §2 */
        uint32_t code;        /* kind-specific: fault vector, Ember panic code, limit */
        uint64_t address;     /* faulting address, or 0 */
        uint64_t rip;         /* instruction pointer at the fault */
        uint32_t pid;
    };

- The record goes to the audit log (kind, caller, attempted object, as in `capabilities.md`).
- The parent receives the record through its wait call. Its encoding is decided in §7.
- Register contents are not reported to user programs. A debug dump goes to the serial log only.

## 6. Mapping to existing and planned work

| Milestone | Work |
|---|---|
| M1 (tessera) | IDT gates for 0 to 31 already exist. Change the dispatch: a fault in ring 3 (CS RPL = 3) is not fatal. For this milestone, it goes to a stub that records the reason. |
| M2 (tessera) | Page-fault classification: not present, protection, NX, guard page. Guard pages below stacks. Fixup table for kernel copies. |
| M3 (tessera, shard, ember) | Ring 3 kill path for the kinds in §2. Tests: one QEMU program per kind, each checked for its exact reason. |
| M4 (tesseraOS) | Loader errors mapped to spawn results (§3). Host tests: one bad image per error code, using the shard fixtures. |
| M5 (runtime) | Ember runtime panics map to `PANIC`. The libc abort path maps to `PANIC` or `SEGV` by signal, per §7. |
| M6 (oxinit) | oxinit reads the reason of each child and applies its restart rules. Its issue #1 bugs about fork failure and exit handling are prerequisites. |

## 7. Open decisions

1. **Signals or exit reasons.** Do we expose the kinds as POSIX signals (SIGILL, SIGFPE, SIGSEGV,
   SIGBUS, SIGTRAP, SIGSYS, SIGKILL for timeouts), or as a tessera-native reason, with oxinit
   mapping it? POSIX mapping is needed if oxinit or a libc keeps its current Signal and Kill
   calls. Table in §2 is written for either.
2. **User fault handlers.** Can a program catch its own faults in v1? Proposed: no. The process
   is killed and the reason reported. A handler mechanism comes later, if at all.
3. **Repeat limit default** and whether it resets with time (§4).
4. **W^X.** Reject any image with a writable and executable section, or allow it with a warning?
5. **Entry checks.** Confirm that the entry point must lie in an executable section for v1.
6. **Alignment checking (#AC).** Enabled for user mode or not? It catches real bugs, but some
   code expects unaligned access.
7. **Timeouts.** Is there a CPU budget per process, and what happens when it is exceeded? If
   yes, the value must be part of the manifest.
8. **Wait encoding.** The format of the reason in the parent's wait result: POSIX wait status
   bits (if decision 1 is POSIX), or the `exit_reason` record.
9. **Ember panics.** Should an Ember panic keep exit code 101 (matches the Linux runtime), or
   become a distinct kind (`PANIC` with the Ember code)? Proposed: distinct kind, keep 101 for
   the Linux exit status.

## 8. Tests

- **Host:** the loader error table in §3, with one malformed fixture per row. Each must return
  the mapped spawn result. Extend the existing shard fuzz target to the kernel's copy path.
- **QEMU, one program per kind** (after M3): `ill-insn`, `div-zero`, `priv-insn`, `bad-address`,
  `protection`, `exec-data`, `stack-overflow`, `misaligned` (if decision 6 enables it),
  `bad-syscall`, `bad-pointer`, `denied` past the limit, `program-panic`. Each program ends
  with its exact reason printed on serial, and the test checks that marker.
- **Kernel stays up:** after every user-mode test, the root shell must still answer `help`.
  A kernel fault in any of these tests is a test failure.
