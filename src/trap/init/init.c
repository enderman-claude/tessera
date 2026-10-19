#include "tessera/trap/init.h"
#include "tessera/trap/gdt.h"
#include "tessera/trap/idt.h"
#include "tessera/trap/isr_table.h"

void trap_init(trap_fatal_fn fatal)
{
    gdt_load();
    for (uint32_t v = 0; v < 32; v++) {
        idt_set_gate((uint8_t)v, isr_table[v], GDT_SEL_CODE);
    }
    idt_load();
    trap_set_fatal(fatal);
}
