#ifndef TESSERA_KERNEL_PANIC_H
#define TESSERA_KERNEL_PANIC_H
#include <stdint.h>

/* Panic: plays the crash message in Morse on Caps Lock for `restart_s` seconds, then resets
 * the machine. restart_s is in seconds. -1 means infinite: the message repeats forever and
 * the machine never restarts. 0 means restart at once. Each message is about 20 seconds,
 * so the number of repeats is restart_s / 20. */

#define PANIC_RESTART_INF (-1)
#define PANIC_MESSAGE_SECONDS 20u

/* Number of message repeats for a restart timeout. PANIC_MESSAGE_COUNT_INF for infinite. */
#define PANIC_MESSAGE_COUNT_INF 0xFFFFFFFFu
uint32_t panic_message_count(int32_t restart_s);

void panic_set_restart(int32_t restart_s);
__attribute__((noreturn)) void kernel_panic(void);

#endif
