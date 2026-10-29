#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copy the keyboard into a qmk_firmware checkout, build, and collect the UF2
# in build/.
#
#   ./build.sh            # default keymap
#   ./build.sh via        # VIA keymap (from keymaps/via)
#
# QMK_HOME defaults to ~/qmk_firmware. If the qmk CLI is not on PATH, set
# QMK_VENV to a virtualenv with `pip install qmk -r requirements.txt`.

set -eu

keymap="${1:-default}"
here="$(cd "$(dirname "$0")" && pwd)"
qmk_home="${QMK_HOME:-$HOME/qmk_firmware}"

if [ -n "${QMK_VENV:-}" ]; then
    PATH="$QMK_VENV/bin:$PATH"
fi

rsync -a --delete "$here/keyboards/mntre_v4/" "$qmk_home/keyboards/mntre_v4/"
# Keymaps that upstream QMK would not accept in the keyboard folder (via).
if [ -d "$here/keymaps/$keymap" ]; then
    rsync -a "$here/keymaps/$keymap/" "$qmk_home/keyboards/mntre_v4/keymaps/$keymap/"
fi
# Generated headers (default_keyboard.h) are not refreshed when files are
# added or renamed, so always start from a clean object directory.
rm -rf "$qmk_home/.build/obj_mntre_v4_$keymap"
make -C "$qmk_home" "mntre_v4:$keymap"

mkdir -p "$here/build"
cp "$qmk_home/mntre_v4_$keymap.uf2" "$here/build/"
echo "UF2: $here/build/mntre_v4_$keymap.uf2"
