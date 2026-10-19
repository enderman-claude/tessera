#include "tessera/modload/check_imports.h"
#include "tessera/lib/str_eq.h"

int check_imports(const struct mod_manifest *m, const char *const *exports, uint32_t n_exports,
                  uint32_t *bad_index)
{
    for (uint32_t i = 0; i < m->n_imports; i++) {
        int found = 0;
        for (uint32_t e = 0; e < n_exports && !found; e++) {
            found = str_eq(m->imports[i], exports[e]);
        }
        if (!found) {
            if (bad_index) *bad_index = i;
            return MOD_ERR_UNRESOLVED_IMPORT;
        }
    }
    return MOD_OK;
}
