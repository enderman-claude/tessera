#include "tessera/scheduler/sched.h"

void sched_init(struct sched *s)
{
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++) {
        s->t[i].state = TASK_FREE;
        s->t[i].priority = 0;
        s->t[i].slice_left = 0;
        s->t[i].saved_sp = 0;
    }
    s->current = -1;
    s->last = -1;
}

int sched_create(struct sched *s, uint8_t priority)
{
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++) {
        if (s->t[i].state == TASK_FREE) {
            s->t[i].state = TASK_READY;
            s->t[i].priority = priority;
            s->t[i].slice_left = SCHED_SLICE_TICKS;
            return (int)i;
        }
    }
    return -1;
}

static int valid(const struct sched *s, int32_t id)
{
    return id >= 0 && (uint32_t)id < SCHED_MAX_TASKS && s->t[id].state != TASK_FREE;
}

int sched_block(struct sched *s, int32_t id)
{
    if (!valid(s, id)) return -1;
    if (s->t[id].state == TASK_RUNNING) s->current = -1;
    s->t[id].state = TASK_BLOCKED;
    return 0;
}

int sched_unblock(struct sched *s, int32_t id)
{
    if (!valid(s, id)) return -1;
    if (s->t[id].state == TASK_BLOCKED) {
        s->t[id].state = TASK_READY;
        s->t[id].slice_left = SCHED_SLICE_TICKS;
    }
    return 0;
}

int sched_exit(struct sched *s, int32_t id)
{
    if (!valid(s, id)) return -1;
    if (s->current == id) s->current = -1;
    s->t[id].state = TASK_FREE;
    s->t[id].priority = 0;
    return 0;
}

/* Highest priority among READY tasks. Ties go to the first ready task after `last`,
 * so equal-priority tasks take turns. */
int32_t sched_pick(struct sched *s)
{
    int32_t best = -1;
    for (uint32_t k = 1; k <= SCHED_MAX_TASKS; k++) {
        uint32_t i = (uint32_t)((s->last + (int32_t)k) % (int32_t)SCHED_MAX_TASKS);
        if (s->last < 0) i = (k - 1) % SCHED_MAX_TASKS;
        if (s->t[i].state != TASK_READY && s->t[i].state != TASK_RUNNING) continue;
        if (best < 0 || s->t[i].priority > s->t[best].priority) best = (int32_t)i;
    }
    if (s->current >= 0 && s->t[s->current].state == TASK_RUNNING) {
        s->t[s->current].state = TASK_READY;
    }
    if (best >= 0) {
        s->t[best].state = TASK_RUNNING;
        s->t[best].slice_left = SCHED_SLICE_TICKS;
        s->last = best;
    }
    s->current = best;
    return best;
}

int32_t sched_tick(struct sched *s)
{
    if (s->current >= 0 && s->t[s->current].state == TASK_RUNNING) {
        if (s->t[s->current].slice_left > 0) s->t[s->current].slice_left--;
        if (s->t[s->current].slice_left > 0) return s->current;
    }
    return sched_pick(s);
}
