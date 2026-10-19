#ifndef TESSERA_SHELL_SERIAL_ROOT_H
#define TESSERA_SHELL_SERIAL_ROOT_H

/* Root console on COM1. The serial port is physical access, so commands run as root.
 * Reads lines forever and never returns. */
__attribute__((noreturn)) void serial_root_run(void);

#endif
