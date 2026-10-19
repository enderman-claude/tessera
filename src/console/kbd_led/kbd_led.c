#include "tessera/console/kbd_led.h"
#include "tessera/port/port_io.h"

#define KBD_DATA 0x60
#define KBD_STATUS 0x64
#define KBD_CMD_SET_LEDS 0xED
#define KBD_ACK 0xFA
#define SPIN_LIMIT 100000u

static int wait_input_clear(void)
{
    for (uint32_t i = 0; i < SPIN_LIMIT; i++) {
        if (!(port_inb(KBD_STATUS) & 0x02)) return 0;
    }
    return -1;
}

static int wait_output_full(void)
{
    for (uint32_t i = 0; i < SPIN_LIMIT; i++) {
        if (port_inb(KBD_STATUS) & 0x01) return 0;
    }
    return -1;
}

int kbd_led_set(uint8_t bits)
{
    if (wait_input_clear() != 0) return -1;
    port_outb(KBD_DATA, KBD_CMD_SET_LEDS);
    if (wait_output_full() != 0 || port_inb(KBD_DATA) != KBD_ACK) return -1;
    if (wait_input_clear() != 0) return -1;
    port_outb(KBD_DATA, bits & 0x07);
    if (wait_output_full() != 0 || port_inb(KBD_DATA) != KBD_ACK) return -1;
    return 0;
}
