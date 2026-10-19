#!/usr/bin/env python3
"""Enforce tessera's source boundaries.

Rules:
1. Every unit is a directory under src/<subsystem>/<unit>/ containing exactly
   one <unit>.c, and its public header is include/tessera/<subsystem>/<unit>.h.
2. A unit's .c may include only: its own public header, standard headers
   (<...>), and headers listed in its optional DEPS file (one per line,
   e.g. "boot/efi_types.h").
3. No unit may include a header from another unit's private directory.
   Only public headers under include/tessera/ are shared.
4. Source files stay under 150 lines.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src"
INC = ROOT / "include" / "tessera"
MAX_LINES = 150

# Layer of each subsystem (see docs/spec/layers.md). Lower number = more privileged.
LAYER = {
    "boot": 0,       # L0 firmware boundary
    "port": 1,       # L1 kernel core (port I/O primitives)
    "trap": 1,       # L1 kernel core (GDT, IDT, exception entry)
    "memory": 1,     # L1 kernel core
    "scheduler": 1,  # L1 kernel core (mechanism)
    "console": 1,    # L1 kernel core (serial output)
    "kernel": 2,     # L2 kernel services (entry, halt, morse, root console)
    "shell": 2,      # L2 kernel services (root serial console)
    "syscall": 2,    # L2 kernel services (syscall entry points)
    "modload": 2,    # L2 kernel services (module loader)
    "crash": 1,      # crash helper, reached only through the L2 syscall
}
SHARED = {"lib"}

# The one allowed upward include: boot (L0) enters the kernel core (L1) at a single
# handoff header. See docs/spec/layers.md, "Handoff".
HANDOFF = {("boot", "kernel")}
INCLUDE_RE = re.compile(r'#\s*include\s+"([^"]+)"')

errors = []

def err(msg):
    errors.append(msg)

for unit_dir in sorted(p for p in SRC.glob("*/*") if p.is_dir()):
    subsystem = unit_dir.parent.name
    unit = unit_dir.name
    rel = f"{subsystem}/{unit}"
    c_file = unit_dir / f"{unit}.c"
    header = INC / subsystem / f"{unit}.h"
    if not c_file.exists():
        err(f"{rel}: missing {unit}.c")
        continue
    if not header.exists():
        err(f"{rel}: missing public header include/tessera/{rel}.h")

    deps = set()
    deps_file = unit_dir / "DEPS"
    if deps_file.exists():
        deps = {l.strip() for l in deps_file.read_text().splitlines() if l.strip()}

    allowed = {f"{rel}.h"} | deps
    for f in sorted(unit_dir.iterdir()):
        if f.suffix not in (".c", ".h"):
            continue
        lines = f.read_text().splitlines()
        if len(lines) > MAX_LINES:
            err(f"{f.relative_to(ROOT)}: {len(lines)} lines (max {MAX_LINES})")
        for line in lines:
            m = INCLUDE_RE.search(line)
            if not m:
                continue
            target = m.group(1)
            if target.startswith("tessera/"):
                public = target[len("tessera/"):]
                if public not in allowed:
                    err(f"{f.relative_to(ROOT)}: includes {target} which is not in its allowed set")
                target_sub = public.split("/")[0]
                if ((subsystem, target_sub) not in HANDOFF and subsystem in LAYER
                        and target_sub in LAYER and LAYER[target_sub] > LAYER[subsystem]):
                    err(f"{f.relative_to(ROOT)}: layer violation, {subsystem} (L{LAYER[subsystem]}) "
                        f"includes {target} from {target_sub} (L{LAYER[target_sub]})")
            else:
                err(f"{f.relative_to(ROOT)}: relative include \"{target}\"; use tessera/... paths")

if errors:
    print("layout check FAILED:")
    for e in errors:
        print("  " + e)
    sys.exit(1)
print(f"layout check ok: {len(list(SRC.glob('*/*')))} units")
