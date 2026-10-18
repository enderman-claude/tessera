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
