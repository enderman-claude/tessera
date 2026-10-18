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
