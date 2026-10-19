#ifndef TESSERA_TRAP_IDT_H
#define TESSERA_TRAP_IDT_H
#include <stdint.h>

/* 64-bit interrupt gates, 256 vectors. */
#define IDT_VECTORS 256u
#define IDT_TYPE_INTERRUPT_GATE 0x8Eu   /* present, DPL 0, 64-bit interrupt gate */

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

/* Packs a gate into 16 bytes. Used by idt_set_gate and by host tests. */
void idt_encode(struct idt_entry *e, uint64_t handler, uint16_t selector, uint8_t type_attr);
void idt_set_gate(uint8_t vector, uint64_t handler, uint16_t selector);
void idt_load(void);

#endif
