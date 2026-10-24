# Isolation for determinism

Status: draft for review.

## Purpose
A program's result depends only on what it was given: its manifest, its handles, and the
bytes it receives through them. Two runs with the same inputs produce the same outputs.

This is **not** a security boundary. Isolation here exists so that results are reproducible,
so that bugs reproduce, and so that tests can be trusted. Capability checks and the audit log
(`capabilities.md`) remain, because they make violations visible. They are not required to
resist a program that is trying to escape, and no part of this spec is designed to do so.

## Rules
1. **Nothing is visible by default.** A new process holds no handles except the ones its
   spawner installs (`abi.md`). It cannot see other processes, files, devices, or endpoints
   unless it holds a handle to them.

2. **Every input comes through a handle.** Anything that varies between runs is an input,
   and must be granted explicitly:
   - time: a `clock` handle (see below)
   - random bytes: a `rand` handle (number to be assigned from the reserved range)
   - arguments and environment: fixed by `exec`
   - data from other programs: `endpoint` handles, and nothing else
   - files: none yet. Any file-like access is a handle, granted by the manifest

3. **No ambient time.** `time` is not part of the always-available `core` group. It needs a
   `clock` handle. This changes the `abi.md` table: syscall 2 moves from `core` to `clock`.

4. **Process-local names.** Process ids and handle numbers are assigned in creation order
   inside each program's own view, so the same program produces the same numbers on every
   run. A program never sees a system-wide pid. (Default, see questions.)

5. **No shared state except by grant.** Two processes share memory only through a `mem`
   handle that the spawner granted to both. There is no global namespace to share through.

6. **Failures are visible, not silent.** A call the program does not hold is `NO_SUCH_CALL`,
   and a refused handle is `DENIED`. Both are reported on the console or audit log so a
   nondeterministic dependency is found, not hidden.

## What this means for the spec
- `abi.md`: `time` moves to a `clock` group and needs a clock handle.
- `capabilities.md`: the `core` group drops `time`. The sentence "There is no ambient
  authority" stays, and its reason is now determinism as well as security.
- oxinit: its control socket and config are already explicit inputs, so it needs no change
  for this rule. Services get only the fds, cwd, groups and environment their config names.

## Questions
1. **Process-local pids.** Should each program see its own pids from 1, or should pids be
   global and only handle numbers be per-process? Per-process pids are fully reproducible;
   global pids are simpler to implement.
2. **Clock and randomness.** Should a program get a clock and a random source only when the
   manifest asks, or should a fixed-seed, fixed-time source be the default for test runs?
3. **Console output.** Is console writing an input-output channel that needs a handle, or a
   sink that every program may use, since it cannot affect other programs' results?
4. **What counts as a file.** Does this spec need a file-like handle now, or can it wait
   until the first program needs to read a file?
