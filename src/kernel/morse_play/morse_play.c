#include "tessera/kernel/morse_play.h"
#include "tessera/kernel/morse_encode.h"
#include "tessera/kernel/delay_pit.h"
#include "tessera/console/kbd_led.h"

static struct morse_event events[MORSE_EVENT_MAX];

__attribute__((noreturn)) void morse_play_forever(const char *text, uint32_t unit_ms, uint32_t repeat_gap_units)
{
    int n = morse_encode(text, events, MORSE_EVENT_MAX);
    /* If the text does not fit, play nothing rather than a truncated message. */
    for (;;) {
        for (int i = 0; n > 0 && i < n; i++) {
            kbd_led_set(events[i].on ? KBD_LED_CAPS : 0);
            delay_ms(events[i].units * unit_ms);
        }
        kbd_led_set(0);
        delay_ms(repeat_gap_units * unit_ms);
    }
}

void morse_play_once(const char *text, uint32_t unit_ms)
{
    int n = morse_encode(text, events, MORSE_EVENT_MAX);
    for (int i = 0; n > 0 && i < n; i++) {
        kbd_led_set(events[i].on ? KBD_LED_CAPS : 0);
        delay_ms(events[i].units * unit_ms);
    }
    kbd_led_set(0);
}
