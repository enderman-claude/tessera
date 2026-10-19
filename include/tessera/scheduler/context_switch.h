#ifndef TESSERA_SCHEDULER_CONTEXT_SWITCH_H
#define TESSERA_SCHEDULER_CONTEXT_SWITCH_H
#include <stdint.h>

/* Switches stacks. Saves callee-saved registers and rsp into *old_sp, then loads new_sp.
 * x86-64 System V. Written, NOT yet run: needs a timer and a booted kernel to test. */
void ctx_switch(uint64_t *old_sp, uint64_t new_sp);

/* Builds the initial stack of a new task so the first ctx_switch to it "returns" into entry.
 * stack_top must be 16-byte aligned. Returns the saved stack pointer to pass as new_sp. */
uint64_t ctx_prepare(uint64_t *stack_top, void (*entry)(void));

#endif
