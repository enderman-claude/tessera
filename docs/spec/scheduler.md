# Scheduler

Layer L1 (kernel core, mechanism). Policy is pure code and host tested. Context switching is
written but has not been run.

## Model
- Fixed task slots: `SCHED_MAX_TASKS` (16). Slots are reused after exit.
- Priority 0 to 255. Higher runs first.
- Round robin among equal priorities. A tie goes to the first ready task after the last one chosen.
- Time slice: `SCHED_SLICE_TICKS` (10) timer ticks. The running task is charged one tick per tick.
- Task states: free, ready, running, blocked.
- Idle when nothing is ready: `current = -1`.
- The caller owns the `struct sched`. There are no globals and no allocation.

## Operations (`scheduler/sched`)
- `sched_create(prio)`: new task, ready. Returns id or -1.
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
