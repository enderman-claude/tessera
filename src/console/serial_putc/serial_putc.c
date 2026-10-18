#include "tessera/console/serial_putc.h"

/* Hardware port write is supplied by the platform layer, not this unit. */
void serial_putc(char c, void (*port_write)(uint16_t port, uint8_t value))
{
    port_write(0x3F8, (uint8_t)c); /* COM1 data register */
}
