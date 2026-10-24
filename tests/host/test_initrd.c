/* Host tests for the initrd stub: the contract callers can rely on today. */
#include <stdio.h>
#include <string.h>
#include "tessera/modload/initrd.h"

static int failures;
#define CHECK(cond, name) do { if (cond) printf("ok   %s\n", name); else { printf("FAIL %s\n", name); failures++; } } while (0)

int main(void)
{
    struct efi_memmap map;
    struct initrd img = { 0xDEADBEEF, 0x1000 };
    memset(&map, 0, sizeof map);

    int rc = initrd_locate(&map, &img);
    CHECK(rc == INITRD_NOT_IMPLEMENTED, "stub reports not implemented");
    CHECK(img.base == 0 && img.size == 0, "stub clears the output to { 0, 0 }");
    CHECK(initrd_locate(&map, &img) == INITRD_NOT_IMPLEMENTED, "stub ignores the map");

    CHECK(strcmp(initrd_status_str(INITRD_OK), "ok") == 0, "status text: ok");
    CHECK(strcmp(initrd_status_str(INITRD_NOT_PRESENT), "no boot-media image") == 0, "status text: not present");
    CHECK(strcmp(initrd_status_str(INITRD_NOT_IMPLEMENTED), "initrd discovery not implemented") == 0, "status text: not implemented");
    CHECK(strcmp(initrd_status_str(12345), "unknown") == 0, "status text: unknown");

    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("initrd: all checks passed\n");
    return 0;
}
