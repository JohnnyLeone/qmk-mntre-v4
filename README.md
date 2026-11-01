# QMK for the MNT Reform Keyboard 4.0

QMK firmware for the RP2040-based [MNT Reform](https://mntre.com/) Keyboard
4.0, following MNT's own firmware
([`reform2-keyboard4-fw`](https://source.mnt.re/reform/reform/-/tree/master/reform2-keyboard4-fw)).

`keyboards/mntre_v4` is laid out exactly as it would be in
[qmk_firmware](https://github.com/qmk/qmk_firmware), next to upstream's
`mntre` and `mntre_v3`, with the goal of submitting it there. See
[MAINLINING.md](MAINLINING.md).

## Download

Prebuilt firmware is attached to each
[release](https://github.com/JohnnyLeone/qmk-mntre-v4/releases/latest):

- `mntre_v4-laptop.uf2`: keyboard inside an MNT Reform (system controller
  keys, battery and status pages)
- `mntre_v4-standalone.uf2`: standalone USB keyboard
- `mntre_v4-*-via.uf2`: the same with VIA, plus `mntre_v4.json` for VIA

Every push to `main` also builds these as a workflow artifact on the
[Actions](https://github.com/JohnnyLeone/qmk-mntre-v4/actions) page. See
[Flashing](#flashing) to install one.

## Features

- Active-high 6x14 matrix (custom scan), 80 keys
- Per-key RGB Matrix backlight (80 WS2812), static effects only (see
  [Backlight](#backlight)), off while the host sleeps
- SSD1306 128x32 OLED: MNT logo, Circle menu legend, battery and system
  controller pages, using MNT's font with its battery and logo glyphs
- Reform system controller over UART: power on/off, wake, status, battery
- Circle key: tap for F24, hold for the menu layer, hold 2 s to power on
  while the laptop is off
- USB serial number from the RP2040 flash unique ID (QMK default on ChibiOS)
- Optional VIA keymap

## Building

```sh
./build.sh          # default keymap -> build/mntre_v4_default.uf2
./build.sh via      # VIA keymap     -> build/mntre_v4_via.uf2
```

### Standalone keyboard

Without the Reform system controller (MNT's standalone variant), build with
`REFORM4_SYSCTL=no`. This drops the UART code; the power, wake, battery and
status keys then do nothing, and Circle long-press power-on is disabled:

```sh
./build.sh default REFORM4_SYSCTL=no   # -> build/mntre_v4_default-REFORM4_SYSCTL_no.uf2
```

Other make variables pass through the same way, e.g. `OLED_ENABLE=no`.

`build.sh` copies `keyboards/mntre_v4` (and `keymaps/<keymap>` if present)
into `$QMK_HOME` (default `~/qmk_firmware`) and runs
`make mntre_v4:<keymap>`. Without the `qmk` CLI on `PATH`, use a virtualenv:

```sh
python3 -m venv ~/.venvs/qmk
~/.venvs/qmk/bin/pip install qmk -r ~/qmk_firmware/requirements.txt
QMK_VENV=~/.venvs/qmk ./build.sh
```

CI builds both keymaps against current qmk_firmware on every push and
weekly; the UF2s are attached to each run as artifacts.

## Flashing

Copy the `.uf2` onto the `RPI-RP2` drive, or on macOS use (prefer UF2 copy
over `picotool load`):

```sh
./flash-uf2-macos.sh
```

Enter the bootloader first (the RP2040 shows up as the `RPI-RP2` drive),
any of:

- Hyper + `R`;
- hold Circle and press `X` twice;
- Bootmagic: hold `Esc` while plugging the keyboard in;
- double-tap the RP2040 reset, or the hardware PROG/RESET path.

## Keys

The bottom-left Hyper key holds the Fn layer: media keys on F7-F12,
display brightness on F1/F2, Home/PgUp/PgDn/End on the arrows, and

| Hyper + | Action                    |
|---------|---------------------------|
| `1`     | power on                  |
| `0`     | power off                 |
| `2`     | battery status (OLED)     |
| `3`     | system controller status  |
| `H`     | wake                      |
| `R`     | bootloader                |

The top-right Circle key sends `F24` when tapped. Held, it activates the
Circle layer, which mirrors the MNT firmware's Circle menu; its legend
alternates between two pages on the OLED:

| Circle +    | Action                                     |
|-------------|--------------------------------------------|
| `1` / `0`   | power on / off                             |
| `B`         | battery status                             |
| `S`         | system controller status                   |
| `Space`     | wake                                       |
| `L`         | MNT logo                                   |
| `U`         | reconnect USB                              |
| `R` `R`     | reset the keyboard (press twice)           |
| `X` `X`     | bootloader / firmware update (press twice) |
| `F1`/`F2`   | backlight brightness down / up             |
| `F3`/`F4`   | backlight hue down / up                    |
| `F5`/`F6`   | backlight saturation down / up             |
| `F7`/`F8`   | previous / next backlight effect (Shift reverses) |
| `F9`        | backlight on / off                         |
| `F10`/`F11` | shift the gradient / modifier colour       |

Brightness, hue, saturation and the gradient shift repeat while held.
Lighting changes are saved to flash 3 seconds after the last one, or right
before a reset or bootloader jump (an RP2040 flash write briefly stalls the
chip, USB included).

## Backlight

The effects are solid colour, `alphas_mods` (modifiers in a second colour)
and the left-right and up-down gradients. They are static, and the RGB
Matrix driver in `rgb_driver.c` only sends a frame to the LEDs when a colour
changes.

A steady stream of LED frames disturbs USB on this board: hosts log
repeated `reset full-speed USB device` messages, keys drop and the
backlight flickers or goes dark until the keyboard is replugged. It happens
at any non-zero brightness, so it is not a USB current limit, and slower
edges on the LED data pin did not help. Stock RGB Matrix re-sends every
frame (~60 per second) even for a solid colour, and animated or
key-reactive effects change every frame, so those are disabled. MNT's
firmware only sets static colours as well.

Backlight effect 2 (Alphas/Mods) gives modifiers, Esc, Circle, Tab,
Backspace, Enter, Delete and all four arrow keys the secondary color.
Printable keys, including backtick/tilde and backslash, use the main color.
These lighting groups are fixed by physical position, including in VIA.

Holding Circle alone for 2 seconds powers the laptop on, like the MNT
firmware. It only arms while the laptop is off (no USB host has configured
the keyboard, or the system controller reported power off), so on a running
computer holding Circle just shows the legend.

## Behaviour carried over from the MNT firmware

- Power on is sent twice (`1p\r`), as leftover UART noise after power off
  can swallow the first command.
- Power off darkens the backlight first; power on restores it.
- Typing while the laptop is reported off shows a hint on the OLED.
- The battery page parses the LPC reply to `c\r` (eight cells, current,
  pack voltage, charge, power state) and refreshes every second.

## Hardware

- MCU: RP2040, USB `1209:6D02` (MNT's pid.codes ID)
- Matrix: 6 rows x 14 columns, active high (columns driven high, rows
  pulled down)
  - rows `GP23 GP22 GP21 GP20 GP19 GP18`
  - columns `GP6`-`GP17`, `GP3`, `GP2`
  - MNT's keymap also lists `[3,13]` (Enter) and `[5,5]` (Space), but the
    board has 80 keys like the v3 keyboard; the scan folds those positions
    into `[3,12]` and `[5,4]`.
- 80 WS2812 LEDs on `GP24`, one per key, chained row by row from the top
  left (14, 14, 14, 13, 14, 11)
- SSD1306 128x32 OLED on I2C0 (`GP0` SDA, `GP1` SCL, address `0x3C`)
- System controller UART1 on `GP4` TX / `GP5` RX, 57600 baud (classic
  Reform). Reform Next and Pocket use 115200: change `REFORM4_SYSCTL_BAUD`
  in `config.h`.

## VIA

Build the `via` keymap, then load `via/mntre_v4.json` in VIA (Settings,
enable "Show Design tab", then Design, Load). The definition is generated
by `tools/gen_via.py` from `keyboards/mntre_v4/keyboard.json`; regenerate
it after changing the layout or keycodes:

```sh
python3 tools/gen_via.py keyboards/mntre_v4/keyboard.json > via/mntre_v4.json
```

`tools/gen_layout.py` prints the layout and RGB Matrix LED tables used in
`keyboard.json`.

The VIA keymap lives in `keymaps/via` rather than in the keyboard folder,
as qmk_firmware no longer accepts VIA keymaps.

## Releasing

Pushing a `v*` tag builds all variants and publishes them as a GitHub
release, with SHA256SUMS and the VIA definition:

```sh
git tag v0.2.0
git push origin v0.2.0
```

## License

GPL-2.0-or-later (see [LICENSES](LICENSES)), except
`keyboards/mntre_v4/reform4_font.c`: MNT's OLED font, copied from the MNT
firmware and licensed GPL-3.0-or-later by MNT Research GmbH. Firmware built
with it is therefore GPL-3.0-or-later.

## Not ported

- IQS9150 trackpad (Reform Next).
- Host HID commands for the OLED and backlight (`OLED`, `XRGB`, ... on
  report `x`), used by MNT's desktop tools.
