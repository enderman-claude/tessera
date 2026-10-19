#include "tessera/kernel/panic.h"
#include "tessera/kernel/morse_play.h"
#include "tessera/port/port_io.h"

#define PANIC_TEXT "HELP ME I HAVE CRASHED AND I CANNOT GET UP"
#define PANIC_UNIT_MS 60     /* 20 words per minute */
#define KBD_STATUS 0x64
#define KBD_CMD_RESET 0xFE   /* pulses the CPU reset line */

static int32_t restart_s = PANIC_RESTART_INF;

uint32_t panic_message_count(int32_t s)
{
    if (s < 0) return PANIC_MESSAGE_COUNT_INF; /* -1: repeat forever */
    return (uint32_t)s / PANIC_MESSAGE_SECONDS;
}

void panic_set_restart(int32_t s)
{
    restart_s = s;
}

__attribute__((noreturn)) void kernel_panic(void)
{
    uint32_t count = panic_message_count(restart_s);
    for (uint32_t i = 0; count == PANIC_MESSAGE_COUNT_INF || i < count; i++) {
        morse_play_once(PANIC_TEXT, PANIC_UNIT_MS);
    }
    /* Reset through the keyboard controller. If that does nothing, stop. */
    port_outb(KBD_STATUS, KBD_CMD_RESET);
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
