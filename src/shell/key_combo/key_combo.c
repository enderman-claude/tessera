#include "tessera/shell/key_combo.h"

#define SC_CTRL 0x1D
#define SC_ALT 0x38
#define SC_LSHIFT 0x2A
#define SC_RSHIFT 0x36
#define SC_CAPS 0x3A
#define SC_ENTER 0x1C
#define SC_BREAK 0x80

void key_state_reset(struct key_state *k)
{
    k->ctrl = 0;
    k->alt = 0;
    k->shift = 0;
    k->caps = 0;
}

enum key_event key_feed(struct key_state *k, uint8_t scancode)
{
    uint8_t released = (scancode & SC_BREAK) != 0;
    uint8_t code = scancode & (uint8_t)~SC_BREAK;

    switch (code) {
    case SC_CTRL: k->ctrl = !released; return KEY_NONE;
    case SC_ALT: k->alt = !released; return KEY_NONE;
    case SC_LSHIFT:
    case SC_RSHIFT: k->shift = !released; return KEY_NONE;
    case SC_CAPS:
        if (!released) k->caps = !k->caps;
        return KEY_NONE;
    case SC_ENTER:
        if (!released && k->ctrl && k->alt && k->shift && k->caps) {
            return KEY_COMBO_CRASH;
        }
        return KEY_NONE;
    default:
        return KEY_NONE;
    }
}
