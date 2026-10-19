#ifndef TESSERA_TRAP_ISR_TABLE_H
#define TESSERA_TRAP_ISR_TABLE_H
#include <stdint.h>

/* Entry stubs for CPU exceptions 0..31, defined in assembly (src/trap/isr_table).
 * isr_table[v] is the stub address for vector v. Each stub pushes a dummy error code
 * when the CPU does not, then vector number, then jumps to the common path. */
extern const uint64_t isr_table[32];

/* The register frame built by the common path. Field order matches the push order. */
struct trap_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t vector;
    uint64_t error;
    uint64_t rip, cs, rflags, rsp, ss;
};

#endif
