#ifndef TESSERA_MODLOAD_RUN_INIT_H
#define TESSERA_MODLOAD_RUN_INIT_H
#include "tessera/modload/manifest_parse.h"

typedef int (*mod_init_fn)(void);

#define MOD_STATUS_OK 0
#define MOD_STATUS_SKIPPED 1  /* optional module failed init; boot continued without it */

/* Runs each module's init in the given order. inits[i] belongs to mods[i].
 * status[i] is set to MOD_STATUS_OK or MOD_STATUS_SKIPPED. A required module that fails
 * returns MOD_ERR_REQUIRED_FAILED at once, and boot must halt. */
int run_init(const struct mod_manifest *mods, uint32_t n, const uint32_t *order,
             const mod_init_fn *inits, uint8_t *status);

#endif
