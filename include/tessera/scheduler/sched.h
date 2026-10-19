#ifndef TESSERA_SCHEDULER_SCHED_H
#define TESSERA_SCHEDULER_SCHED_H
#include <stdint.h>

/* Scheduler policy (L1 mechanism). Fixed task slots, round robin among equal effective
 * priority, time slices in ticks. No allocation, no globals: the caller owns the struct.
 *
 * Priority model:
 *   priority   the program's base priority, default 10. Higher is higher. Unbounded (int32).
 *   start      the value the program starts at, default priority + 5 (so 15).
 *   excess     start - priority, above the base. It decays toward 0 each full slice unless
 *              decay is off for that program. Effective priority = priority + excess.
 *   inf        INT32_MAX, the highest priority. -inf is INT32_MIN.
 * Owner may lower their own program's priority. Only root may raise it. */

#define SCHED_MAX_TASKS 16u
#define SCHED_SLICE_TICKS 10u
#define SCHED_DEFAULT_PRIORITY 10
#define SCHED_DEFAULT_START_OFFSET 5
#define SCHED_PRIO_INF INT32_MAX
#define SCHED_PRIO_NEG_INF INT32_MIN

#define SCHED_ERR_DENIED (-2)
#define SCHED_ERR_NEEDS_ROOT (-3)

enum task_state {
    TASK_FREE = 0,
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
};

struct task {
    uint8_t state;
    uint8_t decay;         /* 1 = excess decays toward priority each full slice */
    uint32_t owner;        /* uid of the owner */
    int32_t priority;      /* base priority */
    int32_t excess;        /* start - priority, decays toward 0 */
    uint32_t slice_left;
    uint64_t saved_sp;
};

struct sched {
    struct task t[SCHED_MAX_TASKS];
    int32_t current;   /* running task id, or -1 for idle */
    int32_t last;      /* last task chosen, for round-robin tie-break, or -1 */
};

void sched_init(struct sched *s);

/* Creates a READY task. start defaults to priority + 5 when the caller passes the default
 * macros. Returns the id, or -1 if no slot is free. */
int sched_create(struct sched *s, uint32_t owner, int32_t priority, int32_t start, uint8_t decay);
int sched_create_default(struct sched *s, uint32_t owner);

/* Effective priority = priority + excess, saturated to the int32 range. -1 for a bad id. */
int64_t sched_effective(const struct sched *s, int32_t id);
int32_t sched_base_priority(const struct sched *s, int32_t id);

/* Owner lowers their own program's priority. Root may set any priority. Returns 0,
 * SCHED_ERR_DENIED (not owner, not root), SCHED_ERR_NEEDS_ROOT (raising without root),
 * or -1 for a bad id. */
int sched_set_priority(struct sched *s, int32_t id, uint32_t caller_uid, int caller_is_root,
                       int32_t new_priority);

int sched_block(struct sched *s, int32_t id);
int sched_unblock(struct sched *s, int32_t id);
int sched_exit(struct sched *s, int32_t id);
/* One timer tick. Charges the running task one tick; on a full slice, decays its excess
 * (if enabled) and reschedules. Returns the id to run next, or -1 for idle. */
int32_t sched_tick(struct sched *s);
/* Chooses the next task now. Returns its id or -1. */
int32_t sched_pick(struct sched *s);

#endif
