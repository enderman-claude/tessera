#ifndef TESSERA_KERNEL_MORSE_ENCODE_H
#define TESSERA_KERNEL_MORSE_ENCODE_H
#include <stddef.h>
#include <stdint.h>

/* Morse as a list of on/off events, each lasting `units` time units. Standard timing:
 * dit 1 on, dah 3 on, 1 off between symbols, 3 off between letters, 7 off between words. */
struct morse_event {
    uint8_t on;     /* 1 = signal on, 0 = off */
    uint8_t units;
};

#define MORSE_EVENT_MAX 1024u

/* Encodes text (A-Z, 0-9, space; lowercase is folded). Unknown characters are skipped.
 * Returns the number of events written, or -1 if out is too small. */
int morse_encode(const char *text, struct morse_event *out, size_t cap);

#endif
