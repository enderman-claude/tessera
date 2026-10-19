#ifndef TESSERA_SHELL_KEY_COMBO_H
#define TESSERA_SHELL_KEY_COMBO_H
#include <stdint.h>

/* PS/2 scancode set 1 combo detector for Ctrl+Alt+Shift+Enter with Caps Lock on.
 * Caps Lock state is tracked from its make code, starting off at boot. */

enum key_event {
    KEY_NONE = 0,
    KEY_COMBO_CRASH,
};

struct key_state {
    uint8_t ctrl;
    uint8_t alt;
    uint8_t shift;
    uint8_t caps;
};

void key_state_reset(struct key_state *k);
enum key_event key_feed(struct key_state *k, uint8_t scancode);

#endif
