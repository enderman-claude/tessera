#include "tessera/kernel/delay_pit.h"
#include "tessera/port/port_io.h"

#define PIT_CH2_DATA 0x42
#define PIT_CMD 0x43
#define PIT_GATE 0x61
#define PIT_1MS_COUNT 1193   /* 1193182 Hz / 1000 */

static void wait_one_ms(void)
{
    /* Mode 0 (interrupt on terminal count), lobyte/hibyte, binary, channel 2. */
    port_outb(PIT_CMD, 0xB0);
    port_outb(PIT_CH2_DATA, PIT_1MS_COUNT & 0xFF);
    port_outb(PIT_CH2_DATA, PIT_1MS_COUNT >> 8);
    /* Gate on, speaker off. */
    uint8_t g = port_inb(PIT_GATE);
    port_outb(PIT_GATE, (uint8_t)((g & ~0x02) | 0x01));
    /* OUT2 (bit 5) goes high at terminal count. */
    while (!(port_inb(PIT_GATE) & 0x20)) {
    }
}

void delay_ms(uint32_t ms)
{
    while (ms--) {
        wait_one_ms();
    }
}
