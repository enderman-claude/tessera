#ifndef TESSERA_MODLOAD_CHECK_IMPORTS_H
#define TESSERA_MODLOAD_CHECK_IMPORTS_H
#include "tessera/modload/manifest_parse.h"

/* Every name in m->imports must appear in exports[0..n_exports). Returns MOD_OK, or
 * MOD_ERR_UNRESOLVED_IMPORT. If bad_index is non-NULL it receives the import index. */
int check_imports(const struct mod_manifest *m, const char *const *exports, uint32_t n_exports,
                  uint32_t *bad_index);

#endif
