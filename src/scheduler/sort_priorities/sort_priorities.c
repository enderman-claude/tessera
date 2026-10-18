#include "tessera/scheduler/sort_priorities.h"

/* Insertion sort, descending by priority. Small N, stable. */
void sort_priorities(uint8_t *prio, size_t n)
{
    for (size_t i = 1; i < n; i++) {
        uint8_t key = prio[i];
        size_t j = i;
        while (j > 0 && prio[j - 1] < key) {
            prio[j] = prio[j - 1];
            j--;
        }
        prio[j] = key;
    }
}
