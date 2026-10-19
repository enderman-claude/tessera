# Specifications

Each subsystem has a spec that defines behaviour, inputs and outputs, and what it
does not do. Code comes after the spec for that subsystem is written.

| Spec | Status |
|---|---|
| [conventions.md](conventions.md) | draft |
| [boot.md](boot.md) | draft |
| [console.md](console.md) | draft |
| [lib.md](lib.md) | draft |
| [kernel.md](kernel.md) | draft |
| [memory.md](memory.md) | draft, not yet coded |
| [scheduler.md](scheduler.md) | draft, not yet coded |
| [layers.md](layers.md) | draft: nine layers, L0 to L8 |
| [kernel_modules.md](kernel_modules.md) | draft: L3 kernel modules, built-in and loadable |
| [driver_classes.md](driver_classes.md) | draft: D0 mediated, D1 direct, D2 bus controller |
| [capabilities.md](capabilities.md) | draft: what a layer may reach, and how it is granted |
| [payload_injection.md](payload_injection.md) | draft: raw payload injected into RAM, booted without SHARD or ELF |
| [ram_test.md](ram_test.md) | draft: boot RAM test, pattern 0x42, bad pages mapped out |
| [panic_morse.md](panic_morse.md) | draft: kernel_halt sends the panic message in Morse on Caps Lock |
