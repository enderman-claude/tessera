#include "tessera/console/serial_com1.h"
#include "tessera/console/serial_putc.h"

static void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void serial_com1_puts(const char *s)
{
    for (; *s; s++) {
        if (*s == '\n') {
            serial_putc('\r', outb);
        }
        serial_putc(*s, outb);
    }
}
