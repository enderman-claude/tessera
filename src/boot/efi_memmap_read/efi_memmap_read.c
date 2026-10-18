#include "tessera/boot/efi_memmap_read.h"

EFI_STATUS efi_memmap_read(EFI_SYSTEM_TABLE *st, struct efi_memmap *m)
{
    m->used = m->buf_size;
    return st->BootServices->GetMemoryMap(&m->used, (EFI_MEMORY_DESCRIPTOR *)m->buf,
                                          &m->map_key, &m->desc_size, &m->desc_version);
}
