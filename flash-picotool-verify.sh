#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later

set -eu

uf2="${1:-$(dirname "$0")/build/mntre_v4_default.uf2}"

if [ ! -f "$uf2" ]; then
    echo "UF2 not found: $uf2" >&2
    exit 1
fi

echo "Loading with picotool update+verify+execute: $uf2" >&2
echo "If this fails part-way through, prefer flash-uf2-macos.sh for the next attempt." >&2

picotool load -u -v -x "$uf2"

