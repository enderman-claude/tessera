# Syscall ABI (draft for review)

Status: draft. Written from the author's answers on tessera #1 and oxinit #8: Tessera-native
ABI; fork, exec and a few convenience calls; oxinit is the primary consumer, the other
programs get stubs. Where the author had no preference, the default below is a choice
made here, marked **(default)**, and can be changed before implementation.

## Calling convention (default)
- Entry is the `syscall` instruction from ring 3.
- Syscall number in `rax`. Arguments in `rdi, rsi, rdx, r10, r8, r9` (SysV order, with `r10`
  in place of `rcx`, which `syscall` clobbers).
- Return value in `rax`, signed 64-bit. Zero or positive on success. Negative on error.
- Consistent with `docs/spec/conventions.md`, where kernel functions return negative `int` on error.

## Error model (default)
Errors are negative return values from a fixed table. Nothing else is returned in `rax`
as an error signal, and no side channel (no errno) exists.

| Code | Name | Meaning |
|---|---|---|
| 0 | OK | Success |
| -1 | DENIED | The caller holds no capability for this operation |
| -2 | BAD_HANDLE | The handle is empty, stale, or of the wrong kind |
| -3 | INVALID | An argument is out of range or malformed |
| -4 | NO_SUCH_CALL | The syscall's group is not in the caller's table (see capabilities.md) |
| -5 | STUB | The call exists in the table but is not implemented yet |
| -6 | NO_MEMORY | The kernel could not allocate |
| -7 | TRY_AGAIN | Transient condition (e.g. no free process slot); retry is allowed |

`NO_SUCH_CALL` and `DENIED` are kept apart on purpose: a program cannot probe for calls
it lacks, but it can see when a call it has is refused.

## Capabilities and handles (default)
- **Per-process handle table.** Each process has a table of capabilities, and every syscall
  that acts on an object takes a small integer handle, an index into that table. This matches
  the delegation and revocation rules in `docs/spec/capabilities.md`.
- No call opens anything by path. There is no ambient namespace. A program's initial handles
  are installed by its spawner from its manifest.
- Handles are checked before any side effect. A failed check is audited (see capabilities.md).

This settles the question left open in `capabilities.md` ("per-process tables or a global
object space"). It is a choice made here, not a decision from the author.

## Syscall groups and numbers
Groups follow `capabilities.md`. Numbers are fixed for the lifetime of the ABI, and unused
numbers are reserved, not reused.

| No. | Name | Group | Arguments | Returns |
|---|---|---|---|---|
| 0 | exit | core | status | does not return |
| 1 | yield | core | — | 0 |
| 2 | time | core | clock id, out ptr | 0; time in ns through the out pointer |
| 3 | fork | proc | — | child handle in parent, 0 in child (see below) |
| 4 | exec | proc | program handle, argv ptr, envp ptr | does not return on success |
| 5 | wait | proc | process handle | exit status |
| 6 | kill | proc | process handle, signal | 0. Any signal to PID 0 (the kernel) returns DENIED, even for root, see `kernel.md` |
| 7 | getpid | proc | — | process id |
| 8 | setsid | proc | — | 0; caller becomes leader of a new session and group |
| 9 | close | core | handle | 0 |
| 10 | dup | core | handle | new handle |
| 11 | map | mem | handle (mem), address hint, length | mapped address |
| 12 | unmap | mem | address, length | 0 |
| 13 | send | ipc | endpoint handle, buffer ptr, length | 0 |
| 14 | recv | ipc | endpoint handle, buffer ptr, length | bytes received |
| 15 | reply | ipc | endpoint handle, buffer ptr, length | 0 |
| 16 | console_write | console | buffer ptr, length | bytes written |

Numbers 17–63 are reserved for the groups below, and are not assigned yet.

### fork and exec
- `fork` duplicates the caller's address space and its handle table. Both processes get
  copies of the handles, with the same rights. Returns the child handle in the parent and 0
  in the child, the same shape as POSIX. (default)
- `exec` replaces the caller's image with the SHARD image named by the program handle. argv
  and envp are NUL-terminated arrays of pointers. Handles are kept across exec, except those
  marked close-on-exec. (default)
- Close-on-exec is a per-handle flag set when the handle is created. `dup` copies the flag.
  This is what oxinit needs to keep the control socket out of children.

## Stubs
Groups that are not oxinit's own have their numbers reserved and their entries present in
the table, returning `STUB` until implemented:
- `dev`, `spawn`, `delegate`, `enum`, `bar`, `msi`, `cfg`, `kcall`, `port`, `dma`: driver and
  bus calls. Owned by the driver layers (L4-L6), not by this ABI draft.
- `ipc` (13-15) is listed above for oxinit's control socket, but the endpoint semantics
  (blocking, message size limits) are not written here yet and are stubbed.

## Open questions (not decided here)
1. **Conflict with `spawn`.** `capabilities.md` makes starting a program an L4-only group
   (`spawn`). Here `exec` belongs to `proc`, which oxinit needs as PID 1. Either `exec` needs
   a `spawn` capability held by oxinit, or `proc` is granted to L4 and oxinit. Needs a decision.
2. **Signals.** `kill` takes a signal number, but the set of signals is not defined. Linux's
   numbers would be the easy default, and the author has not said.
3. **`wait` semantics.** Whether `wait` takes one handle or any child, and what the status
   encoding is, is not decided. oxinit's current code expects the Linux wstatus encoding.
4. **Blocking.** Whether `recv` and `wait` block by default, and how timeouts are given, is
   not written.
5. **Shutdown.** `exit` for the last process, and power off or reboot, are not in this table.
   They belong to the `kcall` group, which is stubbed.
