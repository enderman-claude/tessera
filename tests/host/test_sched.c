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

    int a = sched_create(&s), b = sched_create(&s), c = sched_create(&s);
    CHECK(a == 0 && b == 1 && c == 2, "sched: tasks created in slots 0, 1, 2");
    CHECK(sched_priority(&s, a) == 15 && sched_score(&s, a) == SCHED_SCORE_ONE,
          "sched: new program starts at score 1.0, band 15");

    int32_t seq[6];
    for (int i = 0; i < 6; i++) seq[i] = sched_pick(&s);
    CHECK(seq[0] == 0 && seq[1] == 1 && seq[2] == 2 && seq[3] == 0,
          "sched: equal bands take turns (round robin)");

    struct sched d;
    sched_init(&d);
    int old = sched_create(&d);
    /* Use one full slice: score decays 1.0 -> 0.875, band 15 -> 14. */
    sched_pick(&d);
    for (uint32_t i = 0; i < SCHED_SLICE_TICKS; i++) sched_tick(&d);
    CHECK(sched_score(&d, old) == SCHED_SCORE_ONE - (SCHED_SCORE_ONE >> SCHED_DECAY_SHIFT),
          "sched: one slice decays the score by 1/8");
    CHECK(sched_priority(&d, old) == 14, "sched: decayed score maps to band 14");

    /* Many slices: the band falls toward 10 and never below. */
    for (int i = 0; i < 200; i++) {
        sched_pick(&d);
        for (uint32_t k = 0; k < SCHED_SLICE_TICKS; k++) sched_tick(&d);
    }
    CHECK(sched_priority(&d, old) == 10, "sched: long-running program decays to band 10");
    CHECK(sched_score(&d, old) > 0, "sched: score decays but stays above 0");

    /* A fresh program (band 15) beats an aged one (band 10). */
    int fresh = sched_create(&d);
    CHECK(sched_pick(&d) == fresh, "sched: fresh program preempts an aged program");

    struct sched h;
    sched_init(&h);
    int lo = sched_create(&h);
    int hi = sched_create(&h);
    /* Age lo by one slice so hi has the higher band. */
    sched_pick(&h);
    sched_tick(&h);
    for (uint32_t i = 1; i < SCHED_SLICE_TICKS; i++) sched_tick(&h);
    CHECK(sched_pick(&h) == hi, "sched: higher band chosen over an aged task");
    sched_block(&h, hi);
    CHECK(sched_pick(&h) == lo, "sched: blocked task skipped");
    CHECK(sched_priority(&h, lo) == 14, "sched: aged task reports band 14");

    struct sched e;
    sched_init(&e);
    int only = sched_create(&e);
    sched_pick(&e);
    sched_exit(&e, only);
    CHECK(sched_pick(&e) == -1, "sched: exited task is no longer picked");
    CHECK(sched_create(&e) == only, "sched: exited slot is reused");
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
