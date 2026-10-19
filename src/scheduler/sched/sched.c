#include "tessera/scheduler/sched.h"

static int64_t clamp64(int64_t v)
{
    if (v > SCHED_PRIO_INF) return SCHED_PRIO_INF;
    if (v < SCHED_PRIO_NEG_INF) return SCHED_PRIO_NEG_INF;
    return v;
}

/* One exponential decay step toward 0. Rounds the step up so an excess of 1 still reaches 0. */
static int32_t decay_step(int32_t excess)
{
    if (excess > 0) return excess - (excess + 7) / 8;
    if (excess < 0) return excess + (-excess + 7) / 8;
    return 0;
}

void sched_init(struct sched *s)
{
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++) {
        s->t[i].state = TASK_FREE;
        s->t[i].decay = 0;
        s->t[i].owner = 0;
        s->t[i].priority = 0;
        s->t[i].excess = 0;
        s->t[i].slice_left = 0;
        s->t[i].saved_sp = 0;
    }
    s->current = -1;
    s->last = -1;
}

int sched_create(struct sched *s, uint32_t owner, int32_t priority, int32_t start, uint8_t decay)
{
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++) {
        if (s->t[i].state == TASK_FREE) {
            s->t[i].state = TASK_READY;
            s->t[i].decay = decay ? 1 : 0;
            s->t[i].owner = owner;
            s->t[i].priority = priority;
            s->t[i].excess = (int32_t)clamp64((int64_t)start - (int64_t)priority);
            s->t[i].slice_left = SCHED_SLICE_TICKS;
            return (int)i;
        }
    }
    return -1;
}

int sched_create_default(struct sched *s, uint32_t owner)
{
    return sched_create(s, owner, SCHED_DEFAULT_PRIORITY,
                        SCHED_DEFAULT_PRIORITY + SCHED_DEFAULT_START_OFFSET, 1);
}

static int valid(const struct sched *s, int32_t id)
{
    return id >= 0 && (uint32_t)id < SCHED_MAX_TASKS && s->t[id].state != TASK_FREE;
}

int64_t sched_effective(const struct sched *s, int32_t id)
{
    if (!valid(s, id)) return -1;
    return clamp64((int64_t)s->t[id].priority + (int64_t)s->t[id].excess);
}

int32_t sched_base_priority(const struct sched *s, int32_t id)
{
    if (!valid(s, id)) return 0;
    return s->t[id].priority;
}

int sched_set_priority(struct sched *s, int32_t id, uint32_t caller_uid, int caller_is_root,
                       int32_t new_priority)
{
    if (!valid(s, id)) return -1;
    if (!caller_is_root && s->t[id].owner != caller_uid) return SCHED_ERR_DENIED;
    if (!caller_is_root && new_priority > s->t[id].priority) return SCHED_ERR_NEEDS_ROOT;
    s->t[id].priority = new_priority;
    return 0;
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
    return 0;
}

/* Highest effective priority among READY tasks. Ties go to the first ready task after
 * `last`, so equal priorities take turns. */
int32_t sched_pick(struct sched *s)
{
    int32_t best = -1;
    int64_t best_eff = 0;
    for (uint32_t k = 1; k <= SCHED_MAX_TASKS; k++) {
        uint32_t i = s->last < 0 ? (k - 1) % SCHED_MAX_TASKS
                                 : (uint32_t)((s->last + (int32_t)k) % (int32_t)SCHED_MAX_TASKS);
        if (s->t[i].state != TASK_READY && s->t[i].state != TASK_RUNNING) continue;
        int64_t eff = sched_effective(s, (int32_t)i);
        if (best < 0 || eff > best_eff) {
            best = (int32_t)i;
            best_eff = eff;
        }
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
        struct task *cur = &s->t[s->current];
        if (cur->slice_left > 0) cur->slice_left--;
        if (cur->slice_left > 0) return s->current;
        if (cur->decay) cur->excess = decay_step(cur->excess);
    }
    return sched_pick(s);
}
