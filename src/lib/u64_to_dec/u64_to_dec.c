#include "tessera/lib/u64_to_dec.h"

size_t u64_to_dec(uint64_t value, char *out)
{
    char tmp[21];
    size_t n = 0;
    do {
        tmp[n++] = (char)('0' + value % 10);
        value /= 10;
    } while (value);
    for (size_t i = 0; i < n; i++) {
        out[i] = tmp[n - 1 - i];
    }
    out[n] = '\0';
    return n;
}
