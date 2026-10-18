# Kernel modules (layer L3)

A kernel module extends the kernel from inside it. It is not a driver and not a service:
it has no device of its own and no user-facing interface. Its job is to assist the kernel
with a task that is too big for the core and not needed by every boot.

## What kind of thing a module is
- Basic facilities are built in (L2): the capability table, IPC, the loader, and
  console output. Nothing that the kernel needs before it can load a module is a module.
- A module provides an optional facility the kernel can use once it is loaded. Examples:
  - an ACPI table parser
  - a clock source (TSC calibration, HPET)
  - an IOMMU backend (VT-d, AMD-Vi), needed before any L6 DMA driver can bind
  - a framebuffer or extra console backend
  - a CPU feature detector and mitigation switch
  - an interrupt controller backend (APIC, x2APIC)
- Drivers are not modules. A device driver runs in L5 or L6, with capabilities. A module
  runs in ring 0 and is part of the TCB.

## Built-in and loadable
- **Built-in.** Linked into the kernel image. Initialized in a fixed order by L2. Used for
  facilities that every boot needs.
- **Loadable.** A SHARD image from boot media, with a module manifest. Loaded by L2 after
  L2 is up and before L4 starts. Loaded modules cannot be unloaded in v1.

Whether a module is built-in or loadable is a build choice recorded in the configuration.
Kconfig is planned for this: each module has an entry, and the image contains exactly
the modules enabled in `.config`.

## Manifest
Each module has a manifest, separate from its image:
- `name`, `version`
- `requires`: other modules that must init first
- `provides`: hooks it registers (for example `clocksource`, `iommu`, `console`)
- `imports`: kernel exports it calls, by name, from the kernel's export table
- `optional` or `required`. A required module that fails init halts boot.

The module loader resolves `requires` into an init order and refuses cycles.
Imports are resolved against the kernel export table, which is the same mechanism
as the dynamic-linking stub in SHARD v2 (see `shard/docs/spec/dynlink.md`): a module
never names another module's internals, only kernel exports and hook points.

## Rules
1. A module calls only kernel exports listed in its `imports`. The loader checks this
   before the module runs. Undeclared imports are a load error.
2. A module reaches higher layers only through hooks L2 defines. It never calls L4 or
   above and never touches a process's memory directly.
3. A module holds no capabilities of its own. Its authority is ring 0 plus the exports it
   imports, so the import list is the security review surface.
4. Modules run in the kernel's address space. They are trusted. Nothing isolates them
   from the kernel; a buggy module can corrupt the kernel. Keep modules small.
5. Loadable modules come from signed boot media. Signature verification is not built yet,
   so until it is, only modules in the boot image are accepted.
6. A module init that fails is reported on the console and recorded. Optional failures
   are logged and boot continues; required failures halt boot.
7. There is no unload in v1. A module that needs to stop does so by disabling its hooks.

## Export table
The kernel exports a fixed table of functions to modules: memory allocation for kernel
use, page mapping, console output, timer reads, hook registration, and logging. Each
export has a name, a signature, and a version. Adding an export is a kernel change and is
reviewed like one. The export table is the stable interface for modules; internal
kernel layout is not.

## Not yet decided
- Module signing scheme and key management.
- Whether a module may replace a built-in facility (for example, a second clock source).
- Hot loading after boot. v1 loads only during boot.

## Build order
1. Module manifest format and init-order resolution (host test).
2. Built-in module list in the kernel image, initialized in order.
3. Export table with the first three exports (log, console write, timer read).
4. One loadable module from boot media, loaded before L4.
