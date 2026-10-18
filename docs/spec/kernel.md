# Kernel

## kernel_halt
`__attribute__((noreturn)) void kernel_halt(void)`
Disables interrupts and halts forever (`cli; hlt`). Used as the last step of
stage 1 and as the response to any unrecoverable error once the kernel owns the
machine.
