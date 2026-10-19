#include "tessera/modload/init_order.h"
#include "tessera/lib/str_eq.h"

#define MOD_MAX 64

static int find_module(const struct mod_manifest *mods, uint32_t n, const char *name)
{
    for (uint32_t i = 0; i < n; i++) {
        if (str_eq(mods[i].name, name)) return (int)i;
    }
    return -1;
}

static int check_unique(const struct mod_manifest *mods, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = i + 1; j < n; j++) {
            if (str_eq(mods[i].name, mods[j].name)) return MOD_ERR_DUPLICATE;
        }
        for (uint32_t p = 0; p < mods[i].n_provides; p++) {
            for (uint32_t j = 0; j < n; j++) {
                if (j == i) continue;
                for (uint32_t q = 0; q < mods[j].n_provides; q++) {
                    if (str_eq(mods[i].provides[p], mods[j].provides[q])) return MOD_ERR_PROVIDES_CONFLICT;
                }
            }
        }
    }
    return MOD_OK;
}

int init_order(const struct mod_manifest *mods, uint32_t n, uint32_t *order)
{
    if (n > MOD_MAX) return MOD_ERR_TOO_MANY;
    int rc = check_unique(mods, n);
    if (rc != MOD_OK) return rc;

    /* deps[i][j] = 1 if module i requires module j */
    static uint8_t deps[MOD_MAX][MOD_MAX];
    uint32_t pending[MOD_MAX];
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) deps[i][j] = 0;
        pending[i] = 0;
    }
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t r = 0; r < mods[i].n_requires; r++) {
            int j = find_module(mods, n, mods[i].requires[r]);
            if (j < 0) return MOD_ERR_MISSING_DEP;
            if (!deps[i][j]) {
                deps[i][j] = 1;
                pending[i]++;
            }
        }
    }

    uint32_t placed = 0;
    uint8_t done[MOD_MAX] = {0};
    while (placed < n) {
        int pick = -1;
        for (uint32_t i = 0; i < n; i++) {
            if (!done[i] && pending[i] == 0) { pick = (int)i; break; }
        }
        if (pick < 0) return MOD_ERR_CYCLE;
        done[pick] = 1;
        order[placed++] = (uint32_t)pick;
        for (uint32_t i = 0; i < n; i++) {
            if (deps[i][pick]) pending[i]--;
        }
    }
    return MOD_OK;
}
