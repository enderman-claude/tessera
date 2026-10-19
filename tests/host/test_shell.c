/* Host tests for root console command dispatch. The serial I/O loop is hardware and is not tested here. */
#include <stdio.h>
#include "tessera/shell/dispatch.h"
#include "tessera/syscall/debug_crash.h"

static int failures;
#define CHECK(cond, name) do { if (cond) printf("ok   %s\n", name); else { printf("FAIL %s\n", name); failures++; } } while (0)

int main(void)
{
    CHECK(shell_dispatch("") == SHELL_NONE, "shell: empty line");
    CHECK(shell_dispatch("help") == SHELL_HELP, "shell: help");
    CHECK(shell_dispatch("crash") == SHELL_CRASH, "shell: crash is the crash tool");
    CHECK(shell_dispatch("crashh") == SHELL_UNKNOWN, "shell: match is exact");
    CHECK(shell_dispatch("CRASH") == SHELL_UNKNOWN, "shell: commands are case sensitive");
    CHECK(shell_dispatch("rm -rf") == SHELL_UNKNOWN, "shell: anything else is unknown");

    struct principal user = { .uid = 1000, .is_module = 0 };
    CHECK(sys_debug_crash(&user, CRASH_TIMEOUT_DEFAULT) == -1, "shell: non-root caller denied the tool");
    printf("%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
