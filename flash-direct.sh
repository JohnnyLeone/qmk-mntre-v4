#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later

set -eu

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 path/to/firmware.uf2" >&2
    exit 2
fi

uf2="$1"
if [ ! -f "$uf2" ]; then
    echo "UF2 not found: $uf2" >&2
    exit 1
fi

command -v picotool >/dev/null

# Authenticate while the keyboard is still available for the password.
sudo -v
echo "Firmware: $uf2" >&2
echo "Waiting five seconds. Hold Hyper and press R to enter the bootloader." >&2
sleep 5

# Leave the board in the bootloader if loading or verification fails.
sudo picotool load -u -v "$uf2"
sudo picotool reboot
echo "Firmware verified; keyboard reboot requested." >&2

