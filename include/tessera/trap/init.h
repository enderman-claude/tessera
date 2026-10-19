#ifndef TESSERA_TRAP_INIT_H
#define TESSERA_TRAP_INIT_H
#include <stdint.h>
#include "tessera/trap/dispatch.h"

/* Loads the GDT, installs exception gates 0..31 pointing at isr_table, loads the IDT and
 * registers the fatal handler. Interrupts stay disabled. */
void trap_init(trap_fatal_fn fatal);

#endif
