#include "tessera/modload/initrd.h"

int initrd_locate(const struct efi_memmap *m, struct initrd *out)
{
    (void)m;
    out->base = 0;
    out->size = 0;
    return INITRD_NOT_IMPLEMENTED;
}

const char *initrd_status_str(int status)
{
    switch (status) {
    case INITRD_OK: return "ok";
    case INITRD_NOT_PRESENT: return "no boot-media image";
    case INITRD_NOT_IMPLEMENTED: return "initrd discovery not implemented";
    default: return "unknown";
    }
}
