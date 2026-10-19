#ifndef TESSERA_SCHEDULER_SCHED_H
#define TESSERA_SCHEDULER_SCHED_H
#include <stdint.h>

/* Scheduler policy (L1 mechanism). Fixed task slots, priorities 0..255 (higher runs first),
 * round robin among equal priorities, time slices in ticks. No allocation, no globals:
 * the caller owns the struct. Context switching is in scheduler/context_switch. */

#define SCHED_MAX_TASKS 16u
#define SCHED_SLICE_TICKS 10u

enum task_state {
    TASK_FREE = 0,
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
};

struct task {
    uint8_t state;
    uint8_t priority;
    uint32_t slice_left;
    uint64_t saved_sp;
};

struct sched {
    struct task t[SCHED_MAX_TASKS];
    int32_t current;   /* running task id, or -1 for idle */
    int32_t last;      /* last task chosen, for round-robin tie-break, or -1 */
};

void sched_init(struct sched *s);
/* Returns the new task id, or -1 if no slot is free. The task starts READY. */
int sched_create(struct sched *s, uint8_t priority);
int sched_block(struct sched *s, int32_t id);
int sched_unblock(struct sched *s, int32_t id);
int sched_exit(struct sched *s, int32_t id);
/* One timer tick. Charges the running task one tick and reschedules when its slice is used
 * up or it can no longer run. Returns the id of the task to run next, or -1 for idle. */
int32_t sched_tick(struct sched *s);
/* Chooses the next task now (for example after block or exit). Returns its id or -1. */
int32_t sched_pick(struct sched *s);

#endif
