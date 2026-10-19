#ifndef TESSERA_TRAP_DISPATCH_H
#define TESSERA_TRAP_DISPATCH_H
#include "tessera/trap/isr_table.h"

/* Fatal handler, registered by the kernel at init. L1 cannot include L2 (kernel_panic), so
 * the kernel passes its panic in through this hook. */
typedef void (*trap_fatal_fn)(const struct trap_frame *f);
void trap_set_fatal(trap_fatal_fn fn);

/* Called from the assembly common path. Any exception is fatal for now. */
void trap_dispatch(struct trap_frame *f);

#endif
