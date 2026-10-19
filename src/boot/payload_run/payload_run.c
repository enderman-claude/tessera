#include "tessera/boot/payload_run.h"

void payload_run(uint64_t entry)
{
    void (*fn)(void) = (void (*)(void))(uintptr_t)entry;
    fn();
}
