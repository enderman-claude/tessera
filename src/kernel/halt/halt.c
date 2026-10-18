#include "tessera/kernel/halt.h"

void kernel_halt(void)
{
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
