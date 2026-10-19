#include "tessera/shell/dispatch.h"

static int eq(const char *a, const char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

enum shell_cmd shell_dispatch(const char *line)
{
    if (line[0] == '\0') return SHELL_NONE;
    if (eq(line, "help")) return SHELL_HELP;
    if (eq(line, "crash")) return SHELL_CRASH;
    return SHELL_UNKNOWN;
}
