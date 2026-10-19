# Scheduler

Layer L1 (kernel core, mechanism). Policy is pure code and host tested. Context switching is
written but has not been run.

## Model
- Fixed task slots: `SCHED_MAX_TASKS` (16). Slots are reused after exit.
- Time slice: `SCHED_SLICE_TICKS` (10) timer ticks. The running task is charged one tick per tick.
- Task states: free, ready, running, blocked. Idle when nothing is ready (`current = -1`).
- The caller owns the `struct sched`. No globals, no allocation.

## Priority
- **priority**: the program's base priority. Default 10. Higher is higher. Signed 32-bit, so the
  range is unbounded from -inf to inf. `inf` is `INT32_MAX`, `-inf` is `INT32_MIN`.
- **start**: the value the program starts at. Default is priority + 5, so 15. The spawner may set it.
- **excess** = start - priority. Each full slice moves it toward 0 by one eighth, rounded up, so it
  reaches 0 exactly. Unless decay is off for that program.
- **effective** = priority + excess, saturated to the int32 range. Ranking uses this.
- Default program: priority 10, starts at 15, decays to 10.
- A program can start below its priority. Its excess is negative and rises to 0.

## Who may change priority
- The owner may lower the priority of their own program.
- Only root may raise it, or touch a program they do not own.
- Raising without root returns `SCHED_ERR_NEEDS_ROOT`. A non-owner returns `SCHED_ERR_DENIED`.
- Changing priority does not change excess. The program still decays toward its new priority.

## Operations (`scheduler/sched`)
- `sched_create(owner, priority, start, decay)` and `sched_create_default(owner)`.
- `sched_effective`, `sched_base_priority`: read.
- `sched_set_priority(caller_uid, caller_is_root, new)`: enforces the rules above.
- `sched_block` / `sched_unblock`, `sched_exit`.
- `sched_pick()`: chooses now. Round robin among equal effective priority.
- `sched_tick()`: one timer tick. On a full slice, decays excess (if enabled) and reschedules.

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
- Open: whether a spawner may start a program above 15 without root. Currently any spawner may.
- No preemption outside the timer tick, and no blocking I/O.

## Next steps
1. IDT with a timer interrupt that calls `sched_tick` and `ctx_switch` (shares the fault-handler work).
2. Start an idle task and one test task. Check they alternate under QEMU.
