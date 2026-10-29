#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later

set -eu

uf2="${1:-$(dirname "$0")/build/mntre_v4_default.uf2}"
volume="${RPI_RP2_VOLUME:-/Volumes/RPI-RP2}"

if [ ! -f "$uf2" ]; then
    echo "UF2 not found: $uf2" >&2
    exit 1
fi

echo "Put the keyboard into BOOTSEL mode first:" >&2
echo "  - from working QMK: hold Hyper and press R" >&2
echo "  - from broken firmware: use the hardware PROG/RESET path" >&2
echo >&2
echo "Waiting for $volume ..." >&2

while [ ! -d "$volume" ]; do
    sleep 1
done

echo "Copying $(basename "$uf2") to $volume ..." >&2
cp "$uf2" "$volume/"
sync

echo "UF2 copied. The RP2040 should reboot automatically." >&2
echo "macOS may show a disk-ejected warning; that is normal after RP2040 UF2 flashing." >&2

