/* Panic restart arithmetic. The Morse and reset are hardware and are not run here. */
#include <stdio.h>
#include "tessera/kernel/panic.h"

static int failures;
#define CHECK(cond, name) do { if (cond) printf("ok   %s\n", name); else { printf("FAIL %s\n", name); failures++; } } while (0)

int main(void)
{
    CHECK(panic_message_count(-1) == PANIC_MESSAGE_COUNT_INF, "panic: -1 repeats forever");
    CHECK(panic_message_count(0) == 0, "panic: 0 restarts at once, no message");
    CHECK(panic_message_count(19) == 0, "panic: under one message length, no repeat");
    CHECK(panic_message_count(20) == 1, "panic: 20 s is one message");
    CHECK(panic_message_count(39) == 1, "panic: 39 s is still one message");
    CHECK(panic_message_count(40) == 2, "panic: 40 s is two messages");
    CHECK(panic_message_count(200) == 10, "panic: 200 s is ten messages");
    printf("%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
