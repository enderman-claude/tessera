#include "tessera/scheduler/pick_next.h"

/* Returns the index of the highest-priority ready task, or -1 if none. */
int pick_next(const uint8_t *prio, const uint8_t *ready, size_t n)
{
    int best = -1;
    for (size_t i = 0; i < n; i++) {
        if (ready[i] && (best < 0 || prio[i] > prio[best])) {
            best = (int)i;
        }
    }
    return best;
}
