#ifndef TESSERA_MEMORY_RAM_BAD_LIST_H
#define TESSERA_MEMORY_RAM_BAD_LIST_H
#include <stdint.h>

/* Pages that failed the boot RAM test. The allocator must never hand these out. */
#define RAM_PAGE 4096u
#define RAM_BAD_MAX 4096u

void ram_bad_reset(void);
/* Returns 0 on success, -1 if the list is full (the page is then not recorded). */
int ram_bad_add(uint64_t page_addr);
int ram_bad_contains(uint64_t addr);
uint32_t ram_bad_count(void);
uint64_t ram_bad_at(uint32_t i);
uint32_t ram_bad_overflow(void);

#endif
