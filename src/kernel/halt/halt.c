#include "tessera/kernel/halt.h"
#include "tessera/kernel/morse_play.h"

#define PANIC_TEXT "HELP ME I HAVE CRASHED AND I CANNOT GET UP"
#define PANIC_UNIT_MS 60       /* 20 words per minute: a dit is 60 ms, a dah 180 ms */
#define PANIC_REPEAT_GAP 28    /* about 1.7 s of silence before the message repeats */

/* Terminal state. The kernel has nothing else to do, so it sends the message on Caps Lock. */
void kernel_halt(void)
{
    morse_play_forever(PANIC_TEXT, PANIC_UNIT_MS, PANIC_REPEAT_GAP);
}
