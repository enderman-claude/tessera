#ifndef TESSERA_BOOT_PAYLOAD_INJECT_H
#define TESSERA_BOOT_PAYLOAD_INJECT_H
#include <stddef.h>
#include <stdint.h>

/* Raw payload injection. An external injector (QEMU -device loader, a debugger, or a
 * boot script) writes this descriptor and a raw machine-code payload into RAM at
 * TESS_INJECT_ADDR before the kernel starts. No SHARD or ELF parsing is involved.
 *
 * This path is UNVERIFIED in the sense of the layer model: the bytes are trusted because
 * they were injected, not because they were signed. The kernel checks structure and a
 * checksum, and that the bytes lie in conventional RAM. It does not check authorship.
 *
 * Descriptor (32 bytes, little-endian), followed immediately by the payload:
 *   0  magic   "TINJ"
 *   4  version 1
 *   6  reserved, 0
 *   8  size    payload bytes after the descriptor
 *   12 entry   offset of the entry point within the payload
 *   16 checksum 32-bit sum of the payload bytes
 *   20 flags   must be 0
 *   24 reserved, 0
 */

#ifndef TESS_INJECT_ADDR
#define TESS_INJECT_ADDR 0x200000ull
#endif

#define PAYLOAD_DESC_SIZE 32
#define PAYLOAD_VERSION 1
#define PAYLOAD_MAX_SIZE (4u * 1024u * 1024u)

enum payload_status {
    PAYLOAD_OK = 0,
    PAYLOAD_NO_MAGIC = -1,
    PAYLOAD_BAD_VERSION = -2,
    PAYLOAD_BAD_FLAGS = -3,
    PAYLOAD_BAD_SIZE = -4,
    PAYLOAD_BAD_ENTRY = -5,
    PAYLOAD_BAD_CHECKSUM = -6,
    PAYLOAD_NOT_RAM = -7,
};

#endif
