# Isolation for determinism

Status: draft for review.

## Purpose
A program's result depends only on what it was given. Two runs with the same inputs produce
the same outputs. Isolation exists for reproducibility, so bugs reproduce and tests can be
trusted. It is **not** a security boundary. Capability checks and the audit log
(`capabilities.md`) remain to make violations visible. Nothing here is designed to resist a
program that is actively trying to escape.

## Determinism levels
Every program runs at one of two levels, and the level is recorded with its result.

| Level | What it holds | Deterministic? |
|---|---|---|
| **Pure** | Only stdin, stdout, and stderr. Its arguments and environment come from `exec`. | Yes. Same input, same output. |
| **Filesystem** | Pure, plus a filesystem handle it requested at runtime. | Less so. Results depend on files outside the run. |

A program starts at Pure. It moves to Filesystem only by asking for filesystem access.

## Rules
1. **Nothing is visible by default.** A new process holds stdin, stdout, and stderr, and
   nothing else. It cannot see other processes, files, devices, or endpoints.

2. **Stdin and stdout are the default inputs and outputs.** They are granted at start and
   need no request. A program that only reads stdin and writes stdout stays Pure.

3. **Filesystem access is requested, and granted without a prompt.** A program that needs
   files asks for a filesystem handle at runtime. The grant is automatic: no user prompt,
   no dialog. The asking itself is the only gate, and the program is then at the Filesystem
   level. The request is recorded in the audit log, so the change in level is visible.

4. **Every other input comes through a handle.** The following vary between runs, so each
   is an explicit input:
   - time: a `clock` handle (see `abi.md`)
   - random bytes: a `rand` handle (number to be assigned from the reserved range)
   - shared memory: a `mem` handle granted by a spawner
   - messages from other programs: `endpoint` handles

5. **No ambient time.** `time` is not in the always-available `core` group. It needs a
   `clock` handle.

6. **Process-local names.** Process ids and handle numbers are assigned in creation order
   inside each program's own view, so the same program produces the same numbers on every
   run. (Default, see questions.)

7. **Failures are visible.** A call the program does not hold is `NO_SUCH_CALL`. A refused
   handle is `DENIED`. Both are reported on the console or in the audit log, so a
   nondeterministic dependency is found, not hidden.

## Filesystem access
- Requested through a `fs` syscall group. Number reserved in the range 17-63 of `abi.md`.
- Granted without a prompt. The grant is a handle, with no path-based opening: the program
  receives a handle to a filesystem scope, not a global namespace.
- Using it moves the program to the Filesystem level for the rest of its run. It cannot
  return to Pure.

## What this means for the other specs
- `abi.md`: `time` moves to a `clock` group. Stdin, stdout, stderr are granted at `exec`,
  not by a syscall. A `fs` group is reserved.
- `capabilities.md`: a `fs` capability kind is added, granted by request rather than by
  manifest. "No ambient authority" stays. Determinism is now a stated reason for it, along
  with security.
- oxinit: a service that only reads its stdin and writes its log is Pure. A service whose
  config names a path it reads is Filesystem. Its level should appear in `oxictl status`.

## Questions
1. **Scope of a filesystem handle.** Does a grant cover the whole filesystem, or a directory
   the program names when it asks? Narrower scope keeps results more reproducible.
2. **Read and write.** Should a request say read-only or read-write? A read-only program
   can still depend on files, but it cannot change them for other runs.
3. **Level in output.** Should every result carry its level (Pure or Filesystem), so a
   reader knows which results are reproducible? Or only the audit log?
4. **Process-local pids.** Should pids start from 1 in each program's own view, or be global
   with only handle numbers per-process?
5. **Clock and randomness.** Should test runs get a fixed clock and seed by default?
