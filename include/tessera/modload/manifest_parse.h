#ifndef TESSERA_MODLOAD_MANIFEST_PARSE_H
#define TESSERA_MODLOAD_MANIFEST_PARSE_H
#include <stddef.h>
#include <stdint.h>

/* Module manifest. Text format, one key per line:
 *   name=acpi
 *   version=1
 *   requires=pci,timer
 *   provides=acpi
 *   imports=log_write,mem_alloc
 *   required=1
 * Unknown keys are errors. Lists are comma-separated, no spaces. */

#define MOD_NAME_MAX 32
#define MOD_LIST_MAX 8
#define MOD_IMPORT_MAX 16

enum mod_status {
    MOD_OK = 0,
    MOD_ERR_SYNTAX = -1,
    MOD_ERR_UNKNOWN_KEY = -2,
    MOD_ERR_NAME_MISSING = -3,
    MOD_ERR_TOO_LONG = -4,
    MOD_ERR_TOO_MANY = -5,
    MOD_ERR_MISSING_DEP = -6,
    MOD_ERR_CYCLE = -7,
    MOD_ERR_DUPLICATE = -8,
    MOD_ERR_UNRESOLVED_IMPORT = -9,
    MOD_ERR_PROVIDES_CONFLICT = -10,
};

struct mod_manifest {
    char name[MOD_NAME_MAX];
    uint32_t version;
    char requires[MOD_LIST_MAX][MOD_NAME_MAX];
    uint32_t n_requires;
    char provides[MOD_LIST_MAX][MOD_NAME_MAX];
    uint32_t n_provides;
    char imports[MOD_IMPORT_MAX][MOD_NAME_MAX];
    uint32_t n_imports;
    int required;
};

/* Parses text into *out. Returns MOD_OK or a negative mod_status. */
int manifest_parse(const char *text, size_t len, struct mod_manifest *out);

#endif
