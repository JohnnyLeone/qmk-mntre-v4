// Copyright 2026 Andreas Hirschauer
// SPDX-License-Identifier: GPL-2.0-or-later

// Same as default, with VIA for editing the keymap at runtime. Load
// via/mntre_v4.json in VIA (Settings > Show Design tab).
// Kept outside keyboards/ since qmk_firmware no longer takes via keymaps;
// build.sh copies it in for "./build.sh via".

#include QMK_KEYBOARD_H

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Circle (top right): tap F24, hold for the Circle layer, long-hold to
    // power on while the laptop is off. Hyper (bottom left): Fn layer.
    [R4_LAYER_BASE] = LAYOUT(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  R4_CIRC,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,
        KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,
        KC_LSFT, KC_DEL,  KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_UP,   KC_RSFT,
        R4_HYPR, KC_LGUI, KC_LALT, KC_SPC,  KC_SPC,           KC_SPC,  KC_RALT, KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT
    ),

    // Hyper: media keys, navigation, system controller, bootloader (R).
    [R4_LAYER_HYPER] = LAYOUT(
        _______, KC_BRID, KC_BRIU, _______, _______, _______, _______, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, _______,
        _______, R4_PWR_ON, R4_BATT, R4_STATUS, _______, _______, _______, _______, _______, _______, R4_PWR_OFF, _______, _______, _______,
        _______, _______, _______, _______, QK_BOOT, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, R4_WAKE, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_PGUP, _______,
        _______, _______, _______, _______, _______,          _______, _______, _______, KC_HOME, KC_PGDN, KC_END
    ),

    // Circle: the MNT firmware's Circle menu as a layer (legend on the OLED).
    // F1-F6 brightness/hue/saturation (repeat while held), F7/F8 effect,
    // F9 toggle, F10/F11 speed. R reset and X bootloader ask for a second press.
    [R4_LAYER_CIRCLE] = LAYOUT(
        _______, RM_VALD, RM_VALU, RM_HUED, RM_HUEU, RM_SATD, RM_SATU, RM_PREV, RM_NEXT, RM_TOGG, RM_SPDD, RM_SPDU, _______, _______,
        _______, R4_PWR_ON, _______, _______, _______, _______, _______, _______, _______, _______, R4_PWR_OFF, _______, _______, _______,
        _______, _______, _______, _______, R4_REBOOT, _______, _______, R4_USB_RST, _______, _______, _______, _______, _______, _______,
        _______, _______, R4_STATUS, _______, _______, _______, _______, _______, _______, R4_LOGO, _______, _______, _______,
        _______, _______, _______, R4_BOOT, _______, _______, R4_BATT, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, R4_WAKE, R4_WAKE,          R4_WAKE, _______, _______, _______, _______, _______
    ),
};
// clang-format on
