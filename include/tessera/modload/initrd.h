#ifndef TESSERA_MODLOAD_INITRD_H
#define TESSERA_MODLOAD_INITRD_H
#include <stdint.h>
#include "tessera/boot/efi_memmap_read.h"

/* Boot-media image that carries loadable modules (and later the first userland image).
 * It is the "loadable module from boot media" step in docs/spec/kernel_modules.md.
 * Stub: the locator exists and reports its status, but finds nothing yet. Real discovery
 * needs the boot loader to hand over a location, which is not designed yet. */

enum initrd_status {
    INITRD_OK = 0,
    INITRD_NOT_PRESENT = -1,
    INITRD_NOT_IMPLEMENTED = -2,
};

struct initrd {
    uint64_t base;   /* physical address of the image, 0 when absent */
    uint64_t size;   /* bytes, 0 when absent */
};

/* Looks for a boot-media image in the memory map m. On any status other than INITRD_OK,
 * *out is set to { 0, 0 }. Returns an initrd_status. */
int initrd_locate(const struct efi_memmap *m, struct initrd *out);

/* Short text for a status, for the boot console. */
const char *initrd_status_str(int status);

#endif
