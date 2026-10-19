#ifndef TESSERA_MODLOAD_EXPORT_TABLE_H
#define TESSERA_MODLOAD_EXPORT_TABLE_H
#include <stdint.h>

/* The fixed set of kernel functions modules may import. Adding a name is a kernel change
 * and is reviewed as one (see docs/spec/kernel_modules.md). */
const char *const *export_names(uint32_t *count_out);

#endif
