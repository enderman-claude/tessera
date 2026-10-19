/* Host tests for the Morse encoder. The keyboard and timing are hardware and are not tested here. */
#include <stdio.h>
#include <string.h>
#include "tessera/kernel/morse_encode.h"

static int failures;
#define CHECK(cond, name) do { if (cond) printf("ok   %s\n", name); else { printf("FAIL %s\n", name); failures++; } } while (0)

static struct morse_event ev[MORSE_EVENT_MAX];

static int enc(const char *s) { return morse_encode(s, ev, MORSE_EVENT_MAX); }

static int same(int n, const struct morse_event *want, int wn)
{
    if (n != wn) return 0;
    for (int i = 0; i < n; i++) {
        if (ev[i].on != want[i].on || ev[i].units != want[i].units) return 0;
    }
    return 1;
}

int main(void)
{
    struct morse_event e_want[] = { {1,1}, {0,3} };
    CHECK(same(enc("E"), e_want, 2), "morse: E is dit then letter gap");

    struct morse_event t_want[] = { {1,3}, {0,3} };
    CHECK(same(enc("T"), t_want, 2), "morse: T is dah then letter gap");

    struct morse_event sos[] = { {1,1},{0,1},{1,1},{0,1},{1,1},{0,3},
                                 {1,3},{0,1},{1,3},{0,1},{1,3},{0,3},
                                 {1,1},{0,1},{1,1},{0,1},{1,1},{0,3} };
    CHECK(same(enc("SOS"), sos, 18), "morse: SOS timing exact");

    struct morse_event et[] = { {1,1},{0,3}, {0,4}, {1,3},{0,3} };
    CHECK(same(enc("E T"), et, 5), "morse: word gap is 7 units between E and T (3 + 4)");

    CHECK(same(enc("e"), e_want, 2), "morse: lowercase folded");
    struct morse_event et2[] = { {1,1},{0,3},{1,3},{0,3} };
    CHECK(same(enc("E#T"), et2, 4), "morse: unknown characters skipped");

    CHECK(enc(" E") == -1, "morse: leading space rejected");

    int n = enc("HELP ME I HAVE CRASHED AND I CANNOT GET UP");
    CHECK(n > 0 && n < (int)MORSE_EVENT_MAX, "morse: panic message encodes within the event budget");
    int alternates = 1;
    for (int i = 0; i < n; i++) {
        if (ev[i].units == 0) alternates = 0;
        if (ev[i].on && ev[i].units != 1 && ev[i].units != 3) alternates = 0;
    }
    CHECK(alternates, "morse: every signal is a dit (1) or dah (3), no zero-length events");

    int total = 0;
    for (int i = 0; i < n; i++) total += ev[i].units;
    CHECK(total > 0, "morse: message has a positive duration");

    struct morse_event tiny[2];
    CHECK(morse_encode("SOS", tiny, 2) == -1, "morse: output buffer too small rejected");

    printf("%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
