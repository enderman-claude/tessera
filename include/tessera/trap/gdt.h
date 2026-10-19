#ifndef TESSERA_TRAP_GDT_H
#define TESSERA_TRAP_GDT_H
#include <stdint.h>

/* Our own GDT: null, 64-bit kernel code (selector 0x08), kernel data (selector 0x10). */
#define GDT_SEL_CODE 0x08
#define GDT_SEL_DATA 0x10

uint64_t gdt_code64_entry(void);
uint64_t gdt_data_entry(void);
/* Loads the GDT and reloads CS and the data segments. Call before gdt_load returns. */
void gdt_load(void);

#endif
