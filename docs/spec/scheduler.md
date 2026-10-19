# Scheduler

Layer L1 (kernel core, mechanism). Policy is pure code and host tested. Context switching is
written but has not been run.

## Model
- Fixed task slots: `SCHED_MAX_TASKS` (16). Slots are reused after exit.
- Default priority policy (replaces fixed priorities):
  - A program starts with score 1.0 (Q16 `65536`), which is band 15.
  - Each full time slice it uses decays the score by 1/8 (`score -= score >> 3`), so the score falls
    exponentially toward 0.
  - The band is the score normalised into 10 to 15, higher is higher priority:
    `band = 10 + score * 5 / 1.0`, clamped to 10..15. Score 1.0 is band 15, score 0 is band 10.
  - Higher band runs first. A new program therefore preempts a long-running one.
- Round robin among equal bands. A tie goes to the first ready task after the last one chosen.
- Time slice: `SCHED_SLICE_TICKS` (10) timer ticks. The running task is charged one tick per tick.
- Task states: free, ready, running, blocked.
- Idle when nothing is ready: `current = -1`.
- The caller owns the `struct sched`. There are no globals and no allocation.

## Operations (`scheduler/sched`)
- `sched_create()`: new program, ready, score 1.0. Returns id or -1.
- `sched_priority(id)` / `sched_score(id)`: band and score, for reporting.
- `sched_block(id)` / `sched_unblock(id)`: a blocked task is skipped until unblocked.
- `sched_exit(id)`: frees the slot.
- `sched_pick()`: chooses now. The previous running task goes back to ready.
- `sched_tick()`: one timer tick. Reschedules when the slice is used up or the task can no longer run.

## Context switch (`scheduler/context_switch`)
- `ctx_switch(&old_sp, new_sp)`: saves rbp, rbx, r12 to r15 and rsp, loads the new stack,
  restores the registers, returns. x86-64 System V.
- `ctx_prepare(stack_top, entry)`: builds the first frame of a new task. The first switch to it
  returns into `entry`. At entry, rsp % 16 == 8, as the ABI requires.
- Host tests check the frame layout. The assembly is not run.

## Not yet done
- No timer interrupt drives `sched_tick`. The PIT is only used as a polled delay today.
- Nothing calls `sched_pick` or `ctx_switch` yet. There are no tasks to run.
- `sort_priorities` and `pick_next` remain as utilities. The scheduler does not use them.
- No preemption outside the timer tick, and no blocking I/O.

## Next steps
1. IDT with a timer interrupt that calls `sched_tick` and `ctx_switch` (shares the fault-handler work).
2. Start an idle task and one test task. Check they alternate under QEMU.
