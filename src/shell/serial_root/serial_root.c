#include "tessera/shell/serial_root.h"
#include "tessera/shell/dispatch.h"
#include "tessera/syscall/debug_crash.h"
#include "tessera/console/serial_com1.h"
#include "tessera/port/port_io.h"
#include "tessera/console/ps2_poll.h"
#include "tessera/shell/key_combo.h"

#define COM1_DATA 0x3F8
#define COM1_LSR 0x3FD
#define LINE_MAX 32

/* Polls COM1 and the keyboard together. Returns 1 with a byte in *c, or 0 if nothing is waiting.
 * A keyboard combo crash is run from here and does not return. */
static int poll_input(char *c, struct key_state *keys)
{
    uint8_t sc;
    if (ps2_poll(&sc)) {
        if (key_feed(keys, sc) == KEY_COMBO_CRASH) {
            static const struct principal root = { .uid = 0, .is_module = 0 };
            serial_com1_puts("\ndebug.crash: keyboard combo, immediate\n");
            (void)sys_debug_crash(&root, 0); /* combo: restart at once */
        }
        return 0;
    }
    if (port_inb(COM1_LSR) & 0x01) {
        *c = (char)port_inb(COM1_DATA);
        return 1;
    }
    return 0;
}

static void serial_putc_char(char c)
{
    char s[2] = { c, 0 };
    serial_com1_puts(s);
}

__attribute__((noreturn)) void serial_root_run(void)
{
    static const struct principal root = { .uid = 0, .is_module = 0 };
    static struct key_state keys;
    key_state_reset(&keys);
    char line[LINE_MAX];
    for (;;) {
        serial_com1_puts("root# ");
        uint32_t n = 0;
        for (;;) {
            char c;
            if (!poll_input(&c, &keys)) continue;
            if (c == '\r' || c == '\n') break;
            if (c == '\b' || c == 0x7F) {
                if (n > 0) { n--; serial_com1_puts("\b \b"); }
                continue;
            }
            if (n < LINE_MAX - 1) {
                line[n++] = c;
                serial_putc_char(c);
            }
        }
        line[n] = '\0';
        serial_com1_puts("\n");
        switch (shell_dispatch(line)) {
        case SHELL_NONE:
            break;
        case SHELL_HELP:
            serial_com1_puts("commands: help, crash\n");
            break;
        case SHELL_CRASH:
            (void)sys_debug_crash(&root, CRASH_TIMEOUT_DEFAULT);
            break;
        case SHELL_UNKNOWN:
            serial_com1_puts("unknown command\n");
            break;
        }
    }
}
