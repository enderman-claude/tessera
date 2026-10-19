#ifndef TESSERA_SCHEDULER_SCHED_H
#define TESSERA_SCHEDULER_SCHED_H
#include <stdint.h>

/* Scheduler policy (L1 mechanism). Fixed task slots, priorities 0..255 (higher runs first),
 * round robin among equal priorities, time slices in ticks. No allocation, no globals:
 * the caller owns the struct. Context switching is in scheduler/context_switch. */

#define SCHED_MAX_TASKS 16u
#define SCHED_SLICE_TICKS 10u

/* Default priority policy.
 * A program starts with score 1.0 (SCORE_ONE). Each full time slice it uses decays the score
 * by 1/8 (score -= score >> 3), so it falls exponentially toward 0.
 * The scheduling band is the score normalised into 10..15, where higher is higher priority:
 * score 1.0 gives band 15, score 0 gives band 10. */
#define SCHED_SCORE_ONE 65536u
#define SCHED_DECAY_SHIFT 3
#define SCHED_BAND_MIN 10u
#define SCHED_BAND_MAX 15u

enum task_state {
    TASK_FREE = 0,
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
};

struct task {
    uint8_t state;
    uint8_t priority;      /* scheduling band, 10..15 */
    uint32_t score;        /* Q16 fixed point, SCHED_SCORE_ONE = 1.0 */
    uint32_t slice_left;
    uint64_t saved_sp;
};

struct sched {
    struct task t[SCHED_MAX_TASKS];
    int32_t current;   /* running task id, or -1 for idle */
    int32_t last;      /* last task chosen, for round-robin tie-break, or -1 */
};

void sched_init(struct sched *s);
/* Returns the new task id, or -1 if no slot is free. The task starts READY at score 1.0 (band 15). */
int sched_create(struct sched *s);
/* Scheduling band (10..15) of a task, or -1 if the id is not a live task. */
int sched_priority(const struct sched *s, int32_t id);
/* Score of a task (Q16), for tests and reporting. */
uint32_t sched_score(const struct sched *s, int32_t id);
int sched_block(struct sched *s, int32_t id);
int sched_unblock(struct sched *s, int32_t id);
int sched_exit(struct sched *s, int32_t id);
/* One timer tick. Charges the running task one tick and reschedules when its slice is used
 * up or it can no longer run. Returns the id of the task to run next, or -1 for idle. */
int32_t sched_tick(struct sched *s);
/* Chooses the next task now (for example after block or exit). Returns its id or -1. */
int32_t sched_pick(struct sched *s);

#endif
