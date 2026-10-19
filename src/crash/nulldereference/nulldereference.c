#include "tessera/crash/nulldereference.h"

void crash_null_dereference(void)
{
    volatile int *p = (volatile int *)0;
    *p = 0x42;
}
