#ifndef TESSERA_LIB_U64_TO_DEC_H
#define TESSERA_LIB_U64_TO_DEC_H
#include <stddef.h>
#include <stdint.h>

/* out must hold at least 21 bytes. Returns length excluding the NUL. */
size_t u64_to_dec(uint64_t value, char *out);

#endif
