#ifndef TESSERA_BOOT_PAYLOAD_RUN_H
#define TESSERA_BOOT_PAYLOAD_RUN_H
#include <stdint.h>

/* Transfers control to an already-validated payload entry. Returns if the payload returns. */
void payload_run(uint64_t entry);

#endif
