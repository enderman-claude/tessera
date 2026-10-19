/* Host tests for GDT and IDT encodings. The table loads (lgdt, lidt) and the exception
 * stubs need the CPU and are not run here. */
#include <stdio.h>
#include <string.h>
#include "tessera/trap/gdt.h"
#include "tessera/trap/idt.h"

static int failures;
#define CHECK(cond, name) do { if (cond) printf("ok   %s\n", name); else { printf("FAIL %s\n", name); failures++; } } while (0)

int main(void)
{
    CHECK(gdt_code64_entry() == 0x00AF9A000000FFFFull, "gdt: 64-bit code descriptor");
    CHECK(gdt_data_entry() == 0x00CF92000000FFFFull, "gdt: data descriptor");
    CHECK(((gdt_code64_entry() >> 53) & 1) == 1, "gdt: L bit set for long mode");
    CHECK(GDT_SEL_CODE == 0x08 && GDT_SEL_DATA == 0x10, "gdt: selectors 0x08 and 0x10");

    struct idt_entry e;
    uint64_t handler = 0xFFFF8000DEADBEEFull;
    idt_encode(&e, handler, GDT_SEL_CODE, IDT_TYPE_INTERRUPT_GATE);
    CHECK(e.offset_low == 0xBEEF, "idt: offset bits 0..15");
    CHECK(e.offset_mid == 0xDEAD, "idt: offset bits 16..31");
    CHECK(e.offset_high == 0xFFFF8000u, "idt: offset bits 32..63");
    CHECK(e.selector == 0x08 && e.type_attr == 0x8E && e.ist == 0, "idt: selector, type 0x8E, no IST");
    CHECK(sizeof e == 16, "idt: entry is 16 bytes");

    printf("%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
