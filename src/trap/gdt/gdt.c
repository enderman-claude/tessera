#include "tessera/trap/gdt.h"

/* 64-bit code: present, DPL 0, code, readable, L=1. Base 0, limit ignored in long mode. */
uint64_t gdt_code64_entry(void)
{
    return 0x00AF9A000000FFFFull;
}

/* Data: present, DPL 0, data, writable, 32-bit default (ignored for data). */
uint64_t gdt_data_entry(void)
{
    return 0x00CF92000000FFFFull;
}

static uint64_t gdt[3] __attribute__((aligned(8)));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void gdt_load(void)
{
    gdt[0] = 0;
    gdt[1] = gdt_code64_entry();
    gdt[2] = gdt_data_entry();
    struct gdt_ptr p = { (uint16_t)(sizeof gdt - 1), (uint64_t)(uintptr_t)gdt };
    __asm__ volatile(
        "lgdt %0\n"
        "pushq %1\n"
        "leaq 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"
        "1:\n"
        "movw %2, %%ds\n"
        "movw %2, %%es\n"
        "movw %2, %%ss\n"
        :
        : "m"(p), "i"(GDT_SEL_CODE), "r"((uint16_t)GDT_SEL_DATA)
        : "rax", "memory");
}
