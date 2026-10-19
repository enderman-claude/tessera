#ifndef TESSERA_KERNEL_ENTRY_H
#define TESSERA_KERNEL_ENTRY_H
#include "tessera/boot/efi_memmap_read.h"

/* L0 -> L1 handoff, the second allowed upward include (see docs/spec/layers.md).
 * Called once after ExitBootServices. Runs the boot RAM test, then the injected payload
 * if payload_ready is set, then halts. Does not return. */
__attribute__((noreturn)) void kernel_main(const struct efi_memmap *m, const uint64_t *keep,
                                           uint32_t n_keep, int payload_ready,
                                           uint64_t payload_entry);

#endif
