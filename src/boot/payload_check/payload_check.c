#include "tessera/boot/payload_check.h"

static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint64_t range_end(uint64_t start, uint64_t len)
{
    return start + len;
}

/* True if [start, start+len) lies entirely inside one conventional-memory descriptor. */
static int in_conventional_ram(const struct efi_memmap *m, uint64_t start, uint64_t len)
{
    if (m->desc_size == 0) return 0;
    uint64_t count = m->used / m->desc_size;
    for (uint64_t i = 0; i < count; i++) {
        const EFI_MEMORY_DESCRIPTOR *d =
            (const EFI_MEMORY_DESCRIPTOR *)(m->buf + i * m->desc_size);
        if (d->Type != EFI_CONVENTIONAL_MEMORY) continue;
        uint64_t lo = d->PhysicalStart;
        uint64_t hi = lo + d->NumberOfPages * 4096u;
        if (start >= lo && range_end(start, len) <= hi && range_end(start, len) >= start) {
            return 1;
        }
    }
    return 0;
}

int payload_check(uint64_t base, const struct efi_memmap *m, uint64_t *entry_out)
{
    /* Do not read anything until the descriptor is known to be RAM. */
    if (!in_conventional_ram(m, base, PAYLOAD_DESC_SIZE)) return PAYLOAD_NOT_RAM;
    const uint8_t *d = (const uint8_t *)(uintptr_t)base;
    if (d[0] != 'T' || d[1] != 'I' || d[2] != 'N' || d[3] != 'J') return PAYLOAD_NO_MAGIC;
    if ((uint32_t)d[4] != PAYLOAD_VERSION || d[5] != 0) return PAYLOAD_BAD_VERSION;
    if (rd32(d + 20) != 0) return PAYLOAD_BAD_FLAGS;

    uint32_t size = rd32(d + 8);
    uint32_t entry = rd32(d + 12);
    if (size == 0 || size > PAYLOAD_MAX_SIZE) return PAYLOAD_BAD_SIZE;
    if (entry >= size) return PAYLOAD_BAD_ENTRY;

    const uint8_t *body = d + PAYLOAD_DESC_SIZE;
    uint32_t sum = 0;
    for (uint32_t i = 0; i < size; i++) sum += body[i];
    if (sum != rd32(d + 16)) return PAYLOAD_BAD_CHECKSUM;

    if (!in_conventional_ram(m, base, PAYLOAD_DESC_SIZE + (uint64_t)size)) return PAYLOAD_NOT_RAM;

    *entry_out = base + PAYLOAD_DESC_SIZE + entry;
    return PAYLOAD_OK;
}

const char *payload_status_str(int status)
{
    switch (status) {
    case PAYLOAD_OK: return "ok";
    case PAYLOAD_NO_MAGIC: return "no payload present";
    case PAYLOAD_BAD_VERSION: return "bad version";
    case PAYLOAD_BAD_FLAGS: return "nonzero flags";
    case PAYLOAD_BAD_SIZE: return "bad size";
    case PAYLOAD_BAD_ENTRY: return "entry outside payload";
    case PAYLOAD_BAD_CHECKSUM: return "checksum mismatch";
    case PAYLOAD_NOT_RAM: return "payload not in conventional RAM";
    default: return "unknown";
    }
}
