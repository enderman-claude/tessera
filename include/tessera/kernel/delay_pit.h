#ifndef TESSERA_KERNEL_DELAY_PIT_H
#define TESSERA_KERNEL_DELAY_PIT_H
#include <stdint.h>

/* Busy-waits for ms milliseconds using PIT channel 2 (polled, no interrupts). */
void delay_ms(uint32_t ms);

#endif
