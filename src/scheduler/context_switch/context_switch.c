#include "tessera/scheduler/context_switch.h"

/* Six callee-saved registers, then the return address. */
__asm__(
    ".text\n"
    ".globl ctx_switch\n"
    "ctx_switch:\n"
    "    pushq %rbp\n"
    "    pushq %rbx\n"
    "    pushq %r12\n"
    "    pushq %r13\n"
    "    pushq %r14\n"
    "    pushq %r15\n"
    "    movq %rsp, (%rdi)\n"
    "    movq %rsi, %rsp\n"
    "    popq %r15\n"
    "    popq %r14\n"
    "    popq %r13\n"
    "    popq %r12\n"
    "    popq %rbx\n"
    "    popq %rbp\n"
    "    ret\n"
);

uint64_t ctx_prepare(uint64_t *stack_top, void (*entry)(void))
{
    /* entry slot at top-16 is 16-byte aligned, so at entry rsp % 16 == 8 as the ABI requires. */
    uint64_t *entry_slot = stack_top - 2;
    uint64_t *sp = entry_slot - 6;   /* six zeroed callee-saved registers below the entry slot */
    for (int i = 0; i < 6; i++) sp[i] = 0;
    *entry_slot = (uint64_t)(uintptr_t)entry;
    return (uint64_t)(uintptr_t)sp;
}
