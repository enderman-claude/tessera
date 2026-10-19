#ifndef TESSERA_KERNEL_MORSE_PLAY_H
#define TESSERA_KERNEL_MORSE_PLAY_H
#include <stdint.h>

/* Plays text on the Caps Lock LED, forever. unit_ms is the length of one time unit.
 * Never returns. */
__attribute__((noreturn)) void morse_play_forever(const char *text, uint32_t unit_ms, uint32_t repeat_gap_units);

/* Plays text once on Caps Lock, then turns the LED off. */
void morse_play_once(const char *text, uint32_t unit_ms);

#endif
