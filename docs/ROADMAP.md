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
- [ ] L1: paging, traps, timer
- [ ] L2: capability table, SHARD loader that installs manifest capabilities, syscall dispatch
- [ ] L3: supervisor starts one L7 program and reports its exit
- [ ] L4: serial driver with MMIO/IRQ/port capabilities only, via IPC
- [ ] L5: DMA, only after the IOMMU path works on hardware
