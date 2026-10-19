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
- [ ] L4: supervisor starts one L8 program and reports its exit
- [ ] L5: serial driver (D0) through kernel calls, via IPC
- [ ] L6: D1 driver with BAR and MSI only, then DMA after the IOMMU path works on hardware
- [ ] L6: D2 bus controller enumerates children through the kernel

## Kernel modules (see docs/spec/kernel_modules.md)
- [x] Module manifest and init-order resolution (host test: make test)
- [x] Export table (log_write, console_write, timer_read, mem_alloc), checked against module imports
- [x] Init runner: runs modules in order, required failure halts, optional failure skips
- [ ] Built-in module list compiled into the kernel image
- [ ] One loadable module from boot media, loaded before L4
