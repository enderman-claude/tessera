#ifndef TESSERA_MEMORY_PMM_H
#define TESSERA_MEMORY_PMM_H
#include <stdint.h>
#include "tessera/boot/ram_scan.h"

#define PMM_PAGE_SIZE 4096u
/* The allocator covers at most 4 GiB of physical address space, starting at the lowest
 * usable RAM. Memory above that is not managed. */
#define PMM_MAX_PAGES (1u << 20)

/* Builds the allocator from the RAM regions. Pages that are not in a region, and pages on
 * the boot RAM test's bad list, are never handed out. Returns the number of free pages, or
 * -1 if the regions contain no usable RAM. Safe to call again to rebuild. */
int pmm_init(const struct ram_region *regions, uint32_t n);

/* Returns the physical address of one free page, or 0 when none is left. */
uint64_t pmm_alloc_page(void);

/* Returns 0 on success. -1 if the address is not a page this allocator manages, was never
 * usable, or is not currently allocated (so a double free is rejected). */
int pmm_free_page(uint64_t addr);

uint32_t pmm_free_count(void);

#endif
