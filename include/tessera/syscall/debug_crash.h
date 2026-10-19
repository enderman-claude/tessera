#ifndef TESSERA_SYSCALL_DEBUG_CRASH_H
#define TESSERA_SYSCALL_DEBUG_CRASH_H
#include <stdint.h>

/* Who is calling. The kernel sets this for every caller; programs cannot set it for themselves. */
struct principal {
    uint32_t uid;       /* 0 = root */
    uint8_t is_module;  /* 1 = a loaded kernel module (L3) */
};

/* Authority rule for debug.crash: root, or a loaded kernel module. Nothing else. */
int debug_crash_authorized(const struct principal *p);

/* Syscall debug.crash. Denied (-1, no side effect) unless debug_crash_authorized.
 * If authorized, the crash is triggered and this does not return. */
int64_t sys_debug_crash(const struct principal *p);

#endif
