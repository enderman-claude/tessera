#include "tessera/scheduler/sched.h"

/* Band for a score: 10 + score * 5 / 1.0, clamped to 10..15. */
static uint8_t band_for(uint32_t score)
{
    uint32_t b = SCHED_BAND_MIN + (score * (SCHED_BAND_MAX - SCHED_BAND_MIN)) / SCHED_SCORE_ONE;
    if (b > SCHED_BAND_MAX) b = SCHED_BAND_MAX;
    return (uint8_t)b;
}

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

int sched_create(struct sched *s)
{
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++) {
        if (s->t[i].state == TASK_FREE) {
            s->t[i].state = TASK_READY;
            s->t[i].score = SCHED_SCORE_ONE;
            s->t[i].priority = band_for(SCHED_SCORE_ONE);
            s->t[i].slice_left = SCHED_SLICE_TICKS;
            return (int)i;
        }
    }
    return -1;
}

int sched_priority(const struct sched *s, int32_t id)
{
    if (id < 0 || (uint32_t)id >= SCHED_MAX_TASKS || s->t[id].state == TASK_FREE) return -1;
    return s->t[id].priority;
}

uint32_t sched_score(const struct sched *s, int32_t id)
{
    if (id < 0 || (uint32_t)id >= SCHED_MAX_TASKS) return 0;
    return s->t[id].score;
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
        struct task *cur = &s->t[s->current];
        if (cur->slice_left > 0) cur->slice_left--;
        if (cur->slice_left > 0) return s->current;
        /* Full slice used: decay the score exponentially and recompute the band. */
        cur->score -= cur->score >> SCHED_DECAY_SHIFT;
        cur->priority = band_for(cur->score);
    }
    return sched_pick(s);
}
