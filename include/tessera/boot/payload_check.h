#ifndef TESSERA_BOOT_PAYLOAD_CHECK_H
#define TESSERA_BOOT_PAYLOAD_CHECK_H
#include "tessera/boot/efi_memmap_read.h"
#include "tessera/boot/payload_inject.h"

/* Checks an injected payload at address base (a physical address, identity mapped at boot).
 * m is the memory map in use. On PAYLOAD_OK, *entry_out is the entry address. */
int payload_check(uint64_t base, const struct efi_memmap *m, uint64_t *entry_out);

/* Short text for a status, for the boot console. */
const char *payload_status_str(int status);

#endif
