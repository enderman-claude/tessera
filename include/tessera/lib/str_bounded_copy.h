#ifndef TESSERA_LIB_STR_BOUNDED_COPY_H
#define TESSERA_LIB_STR_BOUNDED_COPY_H
#include <stddef.h>

/* Copies src[0..len) into dst (capacity cap, NUL-terminated). Returns 0 on success,
 * -1 if the source does not fit with its terminator. */
int str_bounded_copy(char *dst, size_t cap, const char *src, size_t len);

#endif
