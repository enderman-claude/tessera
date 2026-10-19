#ifndef TESSERA_MODLOAD_INIT_ORDER_H
#define TESSERA_MODLOAD_INIT_ORDER_H
#include "tessera/modload/manifest_parse.h"

/* Writes the init order of mods[0..n) into order (indexes into mods).
 * Each module comes after everything it requires. Among ready modules the lowest index
 * goes first, so the order is deterministic. Also checks names are unique and that
 * no two modules provide the same facility. Returns MOD_OK or a negative mod_status. */
int init_order(const struct mod_manifest *mods, uint32_t n, uint32_t *order);

#endif
