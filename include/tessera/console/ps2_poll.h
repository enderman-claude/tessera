#ifndef TESSERA_CONSOLE_PS2_POLL_H
#define TESSERA_CONSOLE_PS2_POLL_H
#include <stdint.h>

/* Polls the PS/2 controller for one keyboard scancode. Returns 1 and sets *sc if a keyboard
 * byte is waiting, 0 otherwise. Mouse bytes (status bit 5) are ignored. */
int ps2_poll(uint8_t *sc);

#endif
