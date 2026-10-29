# MNT Reform Keyboard 4.0

QMK firmware for the RP2040-based MNT Reform Keyboard 4.0: per-key RGB
Matrix backlight, OLED pages (Circle menu legend, battery, system
controller status) and the laptop system controller commands.

* Keyboard Maintainer: [Andreas Hirschauer](https://github.com/JohnnyLeone)
* Hardware Supported: MNT Reform Keyboard 4.0 RP2040 PCB
* Hardware Availability: <https://mntre.com/>

Make example for this keyboard (after copying it into `qmk_firmware`):

    make mntre_v4:default

Flashing example for this keyboard:

    make mntre_v4:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in any of these ways:

* **Bootmagic reset**: hold Esc (top left) while plugging in the keyboard
* **Keycode in layout**: Hyper + R (`QK_BOOT`), or Circle + X twice
* **Physical reset**: double-tap the RP2040 reset
