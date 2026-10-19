#include "tessera/shell/serial_root.h"
#include "tessera/shell/dispatch.h"
#include "tessera/syscall/debug_crash.h"
#include "tessera/console/serial_com1.h"
#include "tessera/port/port_io.h"

#define COM1_DATA 0x3F8
#define COM1_LSR 0x3FD
#define LINE_MAX 32

static uint8_t serial_getc(void)
{
    while (!(port_inb(COM1_LSR) & 0x01)) {
    }
    return port_inb(COM1_DATA);
}

static void serial_putc_char(char c)
{
    char s[2] = { c, 0 };
    serial_com1_puts(s);
}

__attribute__((noreturn)) void serial_root_run(void)
{
    static const struct principal root = { .uid = 0, .is_module = 0 };
    char line[LINE_MAX];
    for (;;) {
        serial_com1_puts("root# ");
        uint32_t n = 0;
        for (;;) {
            char c = (char)serial_getc();
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
            serial_com1_puts("debug.crash: running as root\n");
            (void)sys_debug_crash(&root);
            break;
        case SHELL_UNKNOWN:
            serial_com1_puts("unknown command\n");
            break;
        }
    }
}
