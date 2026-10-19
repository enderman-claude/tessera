/* Host tests for the scheduler policy and the context-switch stack layout. The switch itself
 * is assembly and is not run here. */
#include <stdio.h>
#include "tessera/scheduler/sched.h"
#include "tessera/scheduler/context_switch.h"
#include <stdint.h>

static int failures;
#define CHECK(cond, name) do { if (cond) printf("ok   %s\n", name); else { printf("FAIL %s\n", name); failures++; } } while (0)

static void run_slices(struct sched *s, int n)
{
    for (int i = 0; i < n; i++) {
        sched_pick(s);
        for (uint32_t k = 0; k < SCHED_SLICE_TICKS; k++) sched_tick(s);
    }
}

static void test_policy(void)
{
    struct sched s;
    sched_init(&s);
    CHECK(sched_pick(&s) == -1, "sched: nothing ready, idle");

    int a = sched_create_default(&s, 1000), b = sched_create_default(&s, 1000), c = sched_create_default(&s, 1000);
    CHECK(a == 0 && b == 1 && c == 2, "sched: tasks created in slots 0, 1, 2");
    CHECK(sched_base_priority(&s, a) == 10 && sched_effective(&s, a) == 15,
          "sched: default program has priority 10, starts at 15");

    int32_t seq[6];
    for (int i = 0; i < 6; i++) seq[i] = sched_pick(&s);
    CHECK(seq[0] == 0 && seq[1] == 1 && seq[2] == 2 && seq[3] == 0,
          "sched: equal effective priority takes turns (round robin)");

    struct sched d;
    sched_init(&d);
    int old = sched_create_default(&d, 1000);
    run_slices(&d, 1);
    CHECK(sched_effective(&d, old) == 14, "sched: one full slice decays 15 to 14");
    run_slices(&d, 4);
    CHECK(sched_effective(&d, old) == 10, "sched: decay reaches priority 10 exactly");
    run_slices(&d, 50);
    CHECK(sched_effective(&d, old) == 10, "sched: stays at priority once there");

    struct sched n;
    sched_init(&n);
    int nodecay = sched_create(&n, 1000, 10, 15, 0);
    run_slices(&n, 20);
    CHECK(sched_effective(&n, nodecay) == 15, "sched: decay off keeps the start value");

    struct sched lo;
    sched_init(&lo);
    int below = sched_create(&lo, 1000, 10, -5, 1);
    run_slices(&lo, 1);
    CHECK(sched_effective(&lo, below) == -3, "sched: start below priority rises toward it (-5 to -3)");
    run_slices(&lo, 40);
    CHECK(sched_effective(&lo, below) == 10, "sched: rises all the way to priority 10");

    struct sched pr;
    sched_init(&pr);
    int high = sched_create(&pr, 1000, 10, 10 + 5, 1);
    int inf_task = sched_create(&pr, 1000, SCHED_PRIO_INF, SCHED_PRIO_INF, 0);
    CHECK(sched_pick(&pr) == inf_task, "sched: inf is the highest priority");
    sched_block(&pr, inf_task);
    CHECK(sched_pick(&pr) == high, "sched: blocked inf task skipped");

    CHECK(sched_effective(&pr, inf_task) == SCHED_PRIO_INF, "sched: inf saturates, no overflow");
    struct sched ng;
    sched_init(&ng);
    int neg = sched_create(&ng, 1000, SCHED_PRIO_NEG_INF, SCHED_PRIO_NEG_INF, 0);
    CHECK(sched_effective(&ng, neg) == SCHED_PRIO_NEG_INF, "sched: -inf allowed");

    struct sched ow;
    sched_init(&ow);
    int mine = sched_create_default(&ow, 1000);
    CHECK(sched_set_priority(&ow, mine, 1000, 0, 3) == 0 && sched_base_priority(&ow, mine) == 3,
          "sched: owner lowers own priority");
    CHECK(sched_set_priority(&ow, mine, 1000, 0, 20) == SCHED_ERR_NEEDS_ROOT,
          "sched: owner cannot raise priority");
    CHECK(sched_set_priority(&ow, mine, 1001, 0, 1) == SCHED_ERR_DENIED,
          "sched: other user cannot touch a program");
    CHECK(sched_set_priority(&ow, mine, 1001, 1, 20) == 0 && sched_base_priority(&ow, mine) == 20,
          "sched: root can raise and set anything");

    struct sched e;
    sched_init(&e);
    int only = sched_create_default(&e, 1000);
    sched_pick(&e);
    sched_exit(&e, only);
    CHECK(sched_pick(&e) == -1, "sched: exited task is no longer picked");
    CHECK(sched_create_default(&e, 1000) == only, "sched: exited slot is reused");
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
