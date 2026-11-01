## Changes in v0.1.1

- Alphas/Mods (effect 2) now gives Delete and all four arrow keys the
  secondary color, alongside the modifiers and control keys. Backtick/tilde
  and backslash keep the main color with the other punctuation.

## Which file?

| File | For |
|------|-----|
| `mntre_v4-laptop.uf2` | Keyboard inside an MNT Reform (talks to the system controller: power, battery, status) |
| `mntre_v4-standalone.uf2` | Standalone USB keyboard |
| `mntre_v4-*-via.uf2` | Same, with [VIA](https://usevia.app) support; load `mntre_v4.json` in VIA (Settings, "Show Design tab", Design, Load) |

The laptop build talks to the classic Reform system controller at 57600 baud. Reform Next and Pocket use 115200 and need a build with `REFORM4_SYSCTL_BAUD` changed in `config.h`.

## Flashing

1. Enter the bootloader: Hyper + `R`, hold Circle and press `X` twice, or hold `Esc` while plugging the keyboard in. It shows up as the `RPI-RP2` drive.
2. Copy the `.uf2` file onto that drive. The keyboard reboots by itself.

Check downloads against `SHA256SUMS` (`shasum -a 256 -c SHA256SUMS`).
