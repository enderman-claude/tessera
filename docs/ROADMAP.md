# Roadmap

- [x] Repo and push workflow
- [ ] `boot/`: efi_main prints a line via ConOut and returns
- [ ] `boot/`: read memory map, exit boot services
- [ ] `kernel/`: own GDT and IDT, serial console output
- [ ] `kernel/`: physical page allocator from the memory map
- [ ] `kernel/`: paging and higher-half mapping
- [ ] `kernel/`: timer interrupt and a minimal scheduler
- [ ] `third_party/zstd`: freestanding shim so the boot stub can use it

## Userland (later)
- [ ] Define tessera syscall ABI
- [ ] Port oxinit (enderman-claude/oxinit) as PID 1 after syscall layer and GNAT runtime work

## Layers (see docs/spec/layers.md)
- [ ] Kernel loader: start a SHARD image at user privilege
- [ ] Syscall entry and minimal table (exit, console write)
- [ ] IPC between two user programs
- [ ] Driver space: load a driver with a grant; deliver an interrupt
- [ ] User-space services that own device policy
