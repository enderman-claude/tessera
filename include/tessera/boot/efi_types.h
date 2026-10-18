#ifndef TESSERA_BOOT_EFI_TYPES_H
#define TESSERA_BOOT_EFI_TYPES_H
#include <stdint.h>

typedef uint64_t EFI_STATUS;
typedef void *EFI_HANDLE;
typedef struct { uint32_t _unused; } EFI_SYSTEM_TABLE;

#define EFI_SUCCESS ((EFI_STATUS)0)

#endif
