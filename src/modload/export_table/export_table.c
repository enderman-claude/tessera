#include "tessera/modload/export_table.h"

static const char *const exports[] = {
    "log_write",
    "console_write",
    "timer_read",
    "mem_alloc",
};

const char *const *export_names(uint32_t *count_out)
{
    *count_out = (uint32_t)(sizeof exports / sizeof exports[0]);
    return exports;
}
