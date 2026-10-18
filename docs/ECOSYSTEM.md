# Reusable projects: survey and decisions

Survey of enderman-claude repos, and of external projects considered for tessera.
Decision rule: reuse a project only when it is self-contained and its runtime
needs are ones tessera can provide. Kernel core stays original.

## Own repos

| Repo | What it is | Status | Fit for tessera |
|---|---|---|---|
| oxinit | PID 1 init in Ada (GNAT) with a C meme archive. Reaps zombies, restarts services, control socket. | Working, has a QEMU boot test | **Good as userspace init, later.** Needs a POSIX-ish syscall layer, threads, signals, sockets. Not usable until tessera has userland. |
| ouroboros | Self-hosted compiler design, bootstrapped from a Rust seed. | Design notes only | Not needed now. Could be a language for tessera userland later. |
| bellow | Language where file type comes from a sidecar manifest. | Pre-alpha, no grammar or CLI | Not usable. |
| enderarch | Arch-based live/recovery distro tooling. | Alpha | Not relevant to the kernel. Its initrd scripts are Linux-specific. |
| relaybox | Node.js media-session relay server. | Milestone 1 | Not relevant. |
| inkwell-sketch | Browser drawing app. | Working | Not relevant. |
| hello-world, butler, pintinit, batteryremainingd | Small or private or forks. | Varies | Not relevant. |

## External projects

| Project | Use | Decision |
|---|---|---|
| facebook/zstd | Compression, for boot payloads and images | **Adopted as submodule** at `third_party/zstd`. Needs a small freestanding shim (memcpy, memset, allocator hooks) to build in the kernel. |

## Why oxinit is not the kernel

oxinit is a process manager that runs as PID 1 on top of a kernel. It assumes Linux
userspace: `fork`, `waitpid`, signals, Unix sockets, POSIX shell splitting, and
threads from GNAT's runtime. tessera has none of these yet. Using oxinit means
building tessera's syscall interface first, then porting or running a GNAT runtime
on top of it. That is a later milestone (userland), not part of the kernel core.

## Plan

1. Kernel core (now): boot stub, memory map, paging, interrupts, serial console. No external code except zstd.
2. Userland (later): define tessera's syscall ABI, then adapt oxinit as the first init.
3. Revisit ouroboros only if tessera needs a language for userland tools.
