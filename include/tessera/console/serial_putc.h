#ifndef TESSERA_CONSOLE_SERIAL_PUTC_H
#define TESSERA_CONSOLE_SERIAL_PUTC_H
#include <stdint.h>

void serial_putc(char c, void (*port_write)(uint16_t port, uint8_t value));

#endif
