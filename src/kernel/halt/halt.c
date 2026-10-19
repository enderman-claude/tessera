#include "tessera/kernel/halt.h"
#include "tessera/kernel/panic.h"

/* Terminal state. Panics with the current restart policy (default -1: repeat forever). */
void kernel_halt(void)
{
    kernel_panic();
}
