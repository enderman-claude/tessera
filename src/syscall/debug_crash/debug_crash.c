#include "tessera/syscall/debug_crash.h"
#include "tessera/crash/nulldereference.h"

#define SYS_DENIED (-1)

int debug_crash_authorized(const struct principal *p)
{
    if (p == 0) return 0;
    return p->uid == 0 || p->is_module != 0;
}

int64_t sys_debug_crash(const struct principal *p)
{
    if (!debug_crash_authorized(p)) {
        return SYS_DENIED;
    }
    crash_null_dereference();
    return 0;
}
