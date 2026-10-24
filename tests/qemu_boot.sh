#!/bin/sh
# Boots build/BOOTX64.EFI under QEMU with OVMF and checks the serial log for the expected
# markers. Needs qemu-system-x86_64 and OVMF. Override the firmware paths with OVMF_CODE and
# OVMF_VARS if they are elsewhere.
set -eu
cd "$(dirname "$0")/.."

OVMF_CODE=${OVMF_CODE:-/usr/share/OVMF/OVMF_CODE_4M.fd}
OVMF_VARS=${OVMF_VARS:-/usr/share/OVMF/OVMF_VARS_4M.fd}
TIMEOUT=${QEMU_TIMEOUT:-30}

ESP=$(mktemp -d)
VARS=$(mktemp)
LOG=$(mktemp)
trap 'rm -rf "$ESP" "$VARS" "$LOG"' EXIT

mkdir -p "$ESP/EFI/BOOT"
cp build/BOOTX64.EFI "$ESP/EFI/BOOT/BOOTX64.EFI"
cp "$OVMF_VARS" "$VARS"

# The kernel never powers the machine off, so the timeout is the normal way out.
timeout "$TIMEOUT" qemu-system-x86_64 -machine q35 -m 256M -nographic -no-reboot \
    -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" \
    -drive if=pflash,format=raw,file="$VARS" \
    -drive file=fat:rw:"$ESP",format=raw,media=disk > "$LOG" 2>&1 || true

status=0
for marker in \
    "tessera: boot stage 1" \
    "tessera: leaving boot services" \
    "tessera: ram test patterns 0x42 0xBD" \
    "tessera: bad pages mapped: 0" \
    "root#"
do
    if grep -q "$marker" "$LOG"; then
        echo "  PASS  $marker"
    else
        echo "  FAIL  $marker"
        status=1
    fi
done

if [ "$status" -ne 0 ]; then
    echo "--- serial log ---"
    tr -cd '\11\12\15\40-\176' < "$LOG"
fi
exit "$status"
