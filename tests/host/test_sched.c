/* Host tests for the scheduler policy and the context-switch stack layout. The switch itself
 * is assembly and is not run here. */
#include <stdio.h>
#include "tessera/scheduler/sched.h"
#include "tessera/scheduler/context_switch.h"

static int failures;
#define CHECK(cond, name) do { if (cond) printf("ok   %s\n", name); else { printf("FAIL %s\n", name); failures++; } } while (0)

static void test_policy(void)
{
    struct sched s;
    sched_init(&s);
    CHECK(sched_pick(&s) == -1, "sched: nothing ready, idle");

    int a = sched_create(&s, 5), b = sched_create(&s, 5), c = sched_create(&s, 5);
    CHECK(a == 0 && b == 1 && c == 2, "sched: tasks created in slots 0, 1, 2");

    int32_t seq[6];
    for (int i = 0; i < 6; i++) {
        seq[i] = sched_pick(&s);
    }
    CHECK(seq[0] == 0 && seq[1] == 1 && seq[2] == 2 && seq[3] == 0,
          "sched: equal priorities take turns (round robin)");

    struct sched h;
    sched_init(&h);
    int lo = sched_create(&h, 1);
    int hi = sched_create(&h, 9);
    CHECK(sched_pick(&h) == hi, "sched: higher priority chosen first");
    sched_block(&h, hi);
    CHECK(sched_pick(&h) == lo, "sched: blocked task skipped");
    sched_unblock(&h, hi);
    CHECK(sched_tick(&h) == lo || sched_tick(&h) == hi, "sched: tick keeps some task running");

    struct sched t;
    sched_init(&t);
    int x = sched_create(&t, 3);
    int y = sched_create(&t, 3);
    int32_t first = sched_pick(&t);
    int switched = 0;
    for (int i = 0; i < (int)SCHED_SLICE_TICKS + 1; i++) {
        if (sched_tick(&t) != first) switched = 1;
    }
    CHECK(switched, "sched: time slice expiry switches to the next task");
    CHECK(x == 0 && y == 1, "sched: second task exists for the switch");

    struct sched e;
    sched_init(&e);
    int only = sched_create(&e, 4);
    sched_pick(&e);
    sched_exit(&e, only);
    CHECK(sched_pick(&e) == -1, "sched: exited task is no longer picked");
    CHECK(sched_create(&e, 4) == only, "sched: exited slot is reused");

    CHECK(sched_block(&e, 99) == -1 && sched_exit(&e, -1) == -1, "sched: bad ids rejected");
}

static void dummy_entry(void) {}

static void test_stack(void)
{
    static uint64_t stack[64] __attribute__((aligned(16)));
    uint64_t top = (uint64_t)(uintptr_t)(stack + 64);
    uint64_t sp = ctx_prepare((uint64_t *)(uintptr_t)top, dummy_entry);
    uint64_t *frame = (uint64_t *)(uintptr_t)sp;
    CHECK(frame[6] == (uint64_t)(uintptr_t)dummy_entry, "ctx: entry address sits where ret will pop it");
    CHECK(((sp + 7 * 8) % 16) == 8, "ctx: at entry rsp % 16 == 8, as the SysV ABI requires");
}

int main(void)
{
    test_policy();
    test_stack();
    printf("%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
