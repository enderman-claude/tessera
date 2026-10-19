#include "tessera/modload/run_init.h"

int run_init(const struct mod_manifest *mods, uint32_t n, const uint32_t *order,
             const mod_init_fn *inits, uint8_t *status)
{
    for (uint32_t k = 0; k < n; k++) {
        uint32_t i = order[k];
        status[i] = MOD_STATUS_OK;
        if (inits[i] == 0) {
            continue;
        }
        if (inits[i]() != 0) {
            if (mods[i].required) {
                return MOD_ERR_REQUIRED_FAILED;
            }
            status[i] = MOD_STATUS_SKIPPED;
        }
    }
    return MOD_OK;
}
