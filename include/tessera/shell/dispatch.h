#ifndef TESSERA_SHELL_DISPATCH_H
#define TESSERA_SHELL_DISPATCH_H

/* Root serial console commands. */
enum shell_cmd {
    SHELL_NONE = 0,     /* empty line */
    SHELL_HELP,
    SHELL_CRASH,        /* dedicated crash tool (debug.crash) */
    SHELL_UNKNOWN,
};

/* Maps one line (NUL-terminated, no newline) to a command. Matching is exact. */
enum shell_cmd shell_dispatch(const char *line);

#endif
