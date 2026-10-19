#ifndef TESSERA_CONSOLE_SERIAL_COM1_H
#define TESSERA_CONSOLE_SERIAL_COM1_H

/* Writes a NUL-terminated string to COM1 (0x3F8), using serial_putc. Polling, no interrupts. */
void serial_com1_puts(const char *s);

#endif
