/* Authority rule for debug.crash. The check is tested directly; the crash itself is not run. */
#include <stdio.h>
#include "tessera/syscall/debug_crash.h"

static int failures;
#define CHECK(cond, name) do { if (cond) printf("ok   %s\n", name); else { printf("FAIL %s\n", name); failures++; } } while (0)

int main(void)
{
    struct principal root = { .uid = 0, .is_module = 0 };
    struct principal module = { .uid = 1000, .is_module = 1 };
    struct principal user = { .uid = 1000, .is_module = 0 };
    CHECK(debug_crash_authorized(&root), "auth: root allowed");
    CHECK(debug_crash_authorized(&module), "auth: loaded module allowed");
    CHECK(!debug_crash_authorized(&user), "auth: ordinary user denied");
    CHECK(!debug_crash_authorized(0), "auth: no principal denied");
    CHECK(sys_debug_crash(&user) == -1, "auth: denied call returns -1 and does not crash");
    printf("%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
