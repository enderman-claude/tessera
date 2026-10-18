# Scheduler

Draft, not yet coded.

## Model
- Fixed set of task slots. Each slot has a priority (0 to 255, higher runs first)
  and a ready flag.
- `pick_next` returns the index of the highest-priority ready slot, or -1.
- `sort_priorities` orders a priority array in descending order (used for
  reporting, not for scheduling decisions).

## Not yet defined
- Context switching, timers, preemption.
