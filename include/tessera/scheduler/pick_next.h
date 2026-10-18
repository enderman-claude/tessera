#ifndef TESSERA_SCHEDULER_PICK_NEXT_H
#define TESSERA_SCHEDULER_PICK_NEXT_H
#include <stddef.h>
#include <stdint.h>

int pick_next(const uint8_t *prio, const uint8_t *ready, size_t n);

#endif
