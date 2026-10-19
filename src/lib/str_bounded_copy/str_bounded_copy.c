#include "tessera/lib/str_bounded_copy.h"

int str_bounded_copy(char *dst, size_t cap, const char *src, size_t len)
{
    if (cap == 0 || len >= cap) {
        return -1;
    }
    for (size_t i = 0; i < len; i++) {
        dst[i] = src[i];
    }
    dst[len] = '\0';
    return 0;
}
