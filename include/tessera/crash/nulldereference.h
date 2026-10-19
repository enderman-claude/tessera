#ifndef TESSERA_CRASH_NULLDEREFERENCE_H
#define TESSERA_CRASH_NULLDEREFERENCE_H

/* Deliberate null pointer write. Test builds only (CRASH_TEST=1). Never returns when it
 * faults; if it does not fault, it returns after the write. */
void crash_null_dereference(void);

#endif
