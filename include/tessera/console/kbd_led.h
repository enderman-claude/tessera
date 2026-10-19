#ifndef TESSERA_CONSOLE_KBD_LED_H
#define TESSERA_CONSOLE_KBD_LED_H
#include <stdint.h>

#define KBD_LED_SCROLL 0x01
#define KBD_LED_NUM 0x02
#define KBD_LED_CAPS 0x04

/* Sets the keyboard LEDs through the PS/2 controller (command 0xED). Returns 0 on success,
 * -1 if the controller or keyboard does not respond (no keyboard, or a USB-only machine). */
int kbd_led_set(uint8_t bits);

#endif
