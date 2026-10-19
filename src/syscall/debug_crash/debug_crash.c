#include "tessera/syscall/debug_crash.h"
#include "tessera/crash/nulldereference.h"
#include "tessera/kernel/panic.h"

#define SYS_DENIED (-1)
#define SYS_BAD_TIMEOUT (-2)

int debug_crash_authorized(const struct principal *p)
{
    if (p == 0) return 0;
    return p->uid == 0 || p->is_module != 0;
}

/* restart_s: seconds of Morse before the machine restarts after the crash.
 * -1 repeats forever, 0 restarts at once. */
int64_t sys_debug_crash(const struct principal *p, int32_t restart_s)
{
    if (!debug_crash_authorized(p)) {
        return SYS_DENIED;
    }
    if (restart_s < PANIC_RESTART_INF) {
        return SYS_BAD_TIMEOUT;
    }
    panic_set_restart(restart_s);
    crash_null_dereference();
    return 0; /* not reached if the crash faults */
}
