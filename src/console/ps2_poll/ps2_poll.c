#include "tessera/console/ps2_poll.h"
#include "tessera/port/port_io.h"

#define PS2_DATA 0x60
#define PS2_STATUS 0x64

int ps2_poll(uint8_t *sc)
{
    uint8_t st = port_inb(PS2_STATUS);
    if (!(st & 0x01) || (st & 0x20)) return 0;
    *sc = port_inb(PS2_DATA);
    return 1;
}
