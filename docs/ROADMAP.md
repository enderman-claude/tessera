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

## Raw payload (see docs/spec/payload_injection.md)
- [x] Descriptor, validation (magic, checksum, RAM range), run, boot integration, host tests
- [ ] Allocator reserves the injected payload's pages
- [ ] QEMU run with an injected payload (needs QEMU in the environment)

## Boot RAM test (see docs/spec/ram_test.md)
- [x] Bad-page list, pattern test, scan with kernel exclusions, bitmap apply, serial report (host tests)
- [ ] Second pattern (0xBD) to catch stuck-at-zero bits
- [ ] Allocator calls ram_bad_apply before its first allocation
- [ ] Run in QEMU and check the serial report

## Panic loop (see docs/spec/panic_morse.md)
- [x] Morse encoder, host tested. Caps Lock LED driver and PIT delay written.
- [ ] Run on real hardware or QEMU with a PS/2 keyboard and check the LED

## Crash path
- [x] crash/nulldereference behind CRASH_TEST=1
- [ ] IDT with exception handlers that call kernel_halt, so faults reach the Morse panic loop

## Root console and crash tool
- [x] Root serial console on COM1, `crash` runs the crash tool as root (host tested dispatch)
- [ ] IDT fault handlers that call kernel_halt, so a fault shows the Morse panic

## Scheduler (see docs/spec/scheduler.md)
- [x] Policy: slots, priorities, round robin, slices (host tested)
- [x] Context switch and task stack frame (written; frame host tested)
- [ ] Timer interrupt drives sched_tick and ctx_switch (needs IDT)
- [ ] Idle task plus two test tasks alternate under QEMU
