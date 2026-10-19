#ifndef TESSERA_KERNEL_PORT_IO_H
#define TESSERA_KERNEL_PORT_IO_H
#include <stdint.h>

uint8_t port_inb(uint16_t port);
void port_outb(uint16_t port, uint8_t value);

#endif
