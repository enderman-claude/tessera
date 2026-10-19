# Journal

Newest entries first.

## Session 3
- Restructured kernel sources into one directory per unit (src/<subsystem>/<unit>/).
- Added tools/check_layout.py to enforce unit boundaries; wired into make.
- BOOTX64.EFI builds with -Werror and passes the checker.

## Session 2
- Surveyed all enderman-claude repos. Recorded decisions in docs/ECOSYSTEM.md.
- Decided oxinit is a later userland init, not a kernel component.

## Session 1
- Created repo enderman-claude/tessera.
- Checked toolchain: gcc and ld are present; no mingw, QEMU, or OVMF.
- Confirmed `ld -m i386pep --subsystem 10` produces a PE32+ EFI app.
- Added design, roadmap, and this journal.
- Added zstd as a submodule under `third_party/zstd` (external, not written by us).

## Session 4 (paused, state recorded)
- Stage 1 boot units implemented per docs/spec/boot.md: console, memory map read, usable page count, exit boot services, halt.
- Build now uses -fpic and --dynamicbase so relocations are not needed for the code.
- Open: the PE image has an empty Base Relocation Directory, and a `.comment` section sits
  below the image base. Both are still to be checked/removed.
- Open: menuconfig. kconfiglib installed (pip). Kconfig files for units are not written yet.
- Paused to build the SHARD binary format in its own repo (enderman-claude/shard) and
  add it to tesseraOS as lib/shard. Kernel loader integration is the next step.

## Session 5
- Wrote docs/spec/layers.md: kernel space, driver space, user space, their privilege levels, the rules between them, and the build order.
- Decided drivers run as separate programs (ring 3) with explicit hardware grants, not in the kernel.

## Session 6
- Replaced the three-layer plan with eight layers (L0 firmware boundary to L7 applications).
- Removed driver-space defaults that were too permissive: DMA needs an IOMMU domain (refused otherwise), port I/O is limited to a per-port grant, and there is no trusted-without-IOMMU mode.
- Added docs/spec/capabilities.md: capability kinds, syscall groups, delegation (weaker only), revocation, and the enforcement point in L2.

## Session 7
- Added driver classes (docs/spec/driver_classes.md): D0 mediated, D1 direct (own device BAR, MSI, IOMMU DMA), D2 bus controller (adds enumeration and child delegation).
- Rule: data-path registers belong to the device and may be written directly; config registers belong to the kernel and are never written by drivers.
- Loader refuses D1/D2 without an IOMMU or interrupt remapping. Open: IOMMU-required targets.

## Session 8
- Added L3 kernel modules: ring-0 extensions, built-in or loadable during boot, not drivers and not services. See docs/spec/kernel_modules.md.
- Renumbered layers to nine (L0 to L8). Updated capabilities, driver classes and roadmap references.
- Module rules: imports are an explicit list checked by the loader, no capabilities of their own, trusted and part of the TCB, no unload in v1.

## Session 9
- Implemented module manifest parsing (modload/manifest_parse), init-order resolution with cycle, missing-dependency, duplicate and provides-conflict checks (modload/init_order), and import checking against an export list (modload/check_imports).
- Added str_eq and str_bounded_copy to lib.
- make test runs 24 host checks, all passing.
- Corrected one expected value in a test: the algorithm picks the lowest-index ready module, and the test expectation had been wrong.

## Session 10
- Added export table (modload/export_table) and init runner (modload/run_init). Host tests: 32 checks pass.
- Layer enforcement in tools/check_layout.py: a unit includes only its own or more-privileged layers. The L0 to L1 handoff (boot -> kernel/halt.h) is the single documented exception.
- Negative test confirmed the checker rejects a real violation.

## Session 11
- Added raw payload injection: TINJ descriptor at 0x200000, validated against the memory map, run after ExitBootServices. Gated by RAW_PAYLOAD (default 1).
- Host tests cover valid, bad magic, version, flags, size, entry, checksum, and non-RAM cases, and execute a real x86-64 payload that returns 42.
- The EFI image builds with RAW_PAYLOAD=1 and RAW_PAYLOAD=0. Not yet run in firmware.
- Caveat: the payload is trusted by injection, not by signature. Documented in payload_injection.md.

## Session 12
- Boot RAM test: after ExitBootServices, fill free conventional RAM with 0x42, read back, map bad pages out (memory/ram_test, memory/ram_bad_list, memory/ram_bad_apply, boot/ram_scan).
- kernel/entry (kernel_main) is the second L0->L1 handoff, next to kernel/halt. It runs the test, reports over COM1, runs an accepted payload, and halts.
- Host tests: 24 more checks (RAM test, bitmap apply, scan). make test passes all suites.
- Build: stripped the .comment section after linking. ld still prints its warning, then objcopy removes the section.
- Not yet run in QEMU.
