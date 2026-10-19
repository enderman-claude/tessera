#include "tessera/trap/dispatch.h"
#include "tessera/console/serial_com1.h"
#include "tessera/lib/u64_to_dec.h"

static trap_fatal_fn fatal;

void trap_set_fatal(trap_fatal_fn fn)
{
    fatal = fn;
}

void trap_dispatch(struct trap_frame *f)
{
    char num[24];
    serial_com1_puts("tessera: exception vector ");
    u64_to_dec(f->vector, num);
    serial_com1_puts(num);
    serial_com1_puts(" error ");
    u64_to_dec(f->error, num);
    serial_com1_puts(num);
    serial_com1_puts(" rip ");
    u64_to_dec(f->rip, num);
    serial_com1_puts(num);
    serial_com1_puts("\n");
    if (fatal) {
        fatal(f);
    }
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
