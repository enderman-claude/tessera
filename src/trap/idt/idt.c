#include "tessera/trap/idt.h"
#include "tessera/trap/gdt.h"

static struct idt_entry idt[IDT_VECTORS] __attribute__((aligned(16)));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_encode(struct idt_entry *e, uint64_t handler, uint16_t selector, uint8_t type_attr)
{
    e->offset_low = (uint16_t)(handler & 0xFFFF);
    e->selector = selector;
    e->ist = 0;
    e->type_attr = type_attr;
    e->offset_mid = (uint16_t)((handler >> 16) & 0xFFFF);
    e->offset_high = (uint32_t)(handler >> 32);
    e->reserved = 0;
}

void idt_set_gate(uint8_t vector, uint64_t handler, uint16_t selector)
{
    idt_encode(&idt[vector], handler, selector, IDT_TYPE_INTERRUPT_GATE);
}

void idt_load(void)
{
    struct idt_ptr p = { (uint16_t)(sizeof idt - 1), (uint64_t)(uintptr_t)idt };
    __asm__ volatile("lidt %0" : : "m"(p));
}
