// Copyright 2026 Andreas Hirschauer
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include <stdio.h>

#include "sysctl.h"
#include "usb_device_state.h"
#include "usb_main.h"

_Static_assert(REFORM4_CIRCLE_POWER_ON_MS > 500, "REFORM4_CIRCLE_POWER_ON_MS must exceed the 500ms countdown delay");

// Circle long-press tracking. The countdown starts after this delay so a
// normal hold for the Circle layer does not flash the progress bar.
#define CIRCLE_COUNTDOWN_DELAY_MS 500

static bool     circle_held;
static bool     circle_armed;
static uint16_t circle_pressed_at;

// Backlight switched off by a power-off command, restored on power-on.
static bool backlight_off_for_power;

// Lighting keys repeat while held, like the MNT firmware's menu. Repeats
// skip EEPROM; the final value is stored once on release.
#define LIGHT_REPEAT_DELAY_MS 300
#define LIGHT_REPEAT_INTERVAL_MS 40

static uint16_t light_repeat_keycode;
static uint32_t light_repeat_at;
static bool     light_repeated;

// Risky Circle menu actions need a second press within this window, like
// the MNT firmware's "Accept Risk" confirmation.
#define CONFIRM_WINDOW_MS 3000

static uint16_t confirm_keycode;
static uint32_t confirm_until;

static bool light_step(uint16_t keycode) {
    switch (keycode) {
        case RM_VALU:
            rgb_matrix_increase_val_noeeprom();
            return true;
        case RM_VALD:
            rgb_matrix_decrease_val_noeeprom();
            return true;
        case RM_HUEU:
            rgb_matrix_increase_hue_noeeprom();
            return true;
        case RM_HUED:
            rgb_matrix_decrease_hue_noeeprom();
            return true;
        case RM_SATU:
            rgb_matrix_increase_sat_noeeprom();
            return true;
        case RM_SATD:
            rgb_matrix_decrease_sat_noeeprom();
            return true;
        case RM_SPDU:
            rgb_matrix_increase_speed_noeeprom();
            return true;
        case RM_SPDD:
            rgb_matrix_decrease_speed_noeeprom();
            return true;
        default:
            return false;
    }
}

static void light_repeat_stop(void) {
    if (light_repeated) {
        rgb_matrix_sethsv(rgb_matrix_get_hue(), rgb_matrix_get_sat(), rgb_matrix_get_val());
        rgb_matrix_set_speed(rgb_matrix_get_speed());
    }
    light_repeat_keycode = KC_NO;
    light_repeated       = false;
}

static void track_light_repeat(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        light_repeat_stop();
        // The first step is QMK's own (stored) keypress; repeats follow.
        switch (keycode) {
            case RM_VALU:
            case RM_VALD:
            case RM_HUEU:
            case RM_HUED:
            case RM_SATU:
            case RM_SATD:
            case RM_SPDU:
            case RM_SPDD:
                light_repeat_keycode = keycode;
                light_repeat_at      = timer_read32() + LIGHT_REPEAT_DELAY_MS;
                break;
        }
    } else if (keycode == light_repeat_keycode) {
        light_repeat_stop();
    }
}

static void light_repeat_task(void) {
    if (light_repeat_keycode != KC_NO && timer_expired32(timer_read32(), light_repeat_at)) {
        light_step(light_repeat_keycode);
        light_repeated  = true;
        light_repeat_at = timer_read32() + LIGHT_REPEAT_INTERVAL_MS;
    }
}

// True on the confirming second press; on the first press shows the prompt.
static bool confirmed(uint16_t keycode, const char *key, const char *action) {
    if (confirm_keycode == keycode && !timer_expired32(timer_read32(), confirm_until)) {
        confirm_keycode = KC_NO;
        return true;
    }
    char prompt[22];
    snprintf(prompt, sizeof(prompt), "Press %s again to", key);
    confirm_keycode = keycode;
    confirm_until   = timer_read32() + CONFIRM_WINDOW_MS;
    reform4_oled_message(prompt, action, CONFIRM_WINDOW_MS);
    return false;
}

// Draw a final message right away; the MCU resets before the next OLED task.
static void flush_message(const char *line1, const char *line2) {
    oled_clear();
    oled_set_cursor(0, 1);
    oled_write_ln(line1, false);
    if (line2) {
        oled_write_ln(line2, false);
    }
    oled_render_dirty(true);
}

static void reform4_power_on(void) {
    sysctl_power_on();
    if (backlight_off_for_power) {
        rgb_matrix_enable_noeeprom();
        backlight_off_for_power = false;
    }
    reform4_oled_message("Powering on...", NULL, 2000);
}

static void reform4_power_off(void) {
    // The MNT firmware darkens the backlight before cutting the rails.
    if (rgb_matrix_is_enabled()) {
        rgb_matrix_disable_noeeprom();
        backlight_off_for_power = true;
    }
    sysctl_power_off();
    reform4_oled_message("Powering off...", NULL, 1500);
}

// Long-press power-on only makes sense while the laptop is off: then its
// USB host is down. Connected to a running computer it would only hide the
// Circle legend behind the countdown.
static bool circle_power_on_allowed(void) {
    return sysctl_power_state() == SYSCTL_POWER_OFF || usb_device_state_get_configure_state() != USB_DEVICE_STATE_CONFIGURED;
}

static void track_circle(uint16_t keycode, keyrecord_t *record) {
    if (keycode == R4_CIRC) {
        circle_held = record->event.pressed;
        if (circle_held) {
            circle_pressed_at = record->event.time;
            circle_armed      = circle_power_on_allowed();
        } else {
            circle_armed = false;
            reform4_oled_power_on_progress(0);
        }
    } else if (record->event.pressed) {
        // Any other key while Circle is held is a menu/lighting chord.
        circle_armed = false;
        reform4_oled_power_on_progress(0);
    }
}

static void circle_task(void) {
    if (!circle_held || !circle_armed) {
        return;
    }

    const uint16_t held = timer_elapsed(circle_pressed_at);
    if (held >= REFORM4_CIRCLE_POWER_ON_MS) {
        circle_armed = false;
        reform4_oled_power_on_progress(0);
        reform4_power_on();
    } else if (held >= CIRCLE_COUNTDOWN_DELAY_MS) {
        const uint16_t span = REFORM4_CIRCLE_POWER_ON_MS - CIRCLE_COUNTDOWN_DELAY_MS;
        const uint8_t  pct  = (uint8_t)(1 + (uint32_t)(held - CIRCLE_COUNTDOWN_DELAY_MS) * 99 / span);
        reform4_oled_power_on_progress(pct);
    }
}

void keyboard_post_init_kb(void) {
    sysctl_init();
    reform4_oled_show(R4_OLED_SPLASH, 1500);
    keyboard_post_init_user();
}

void housekeeping_task_kb(void) {
    sysctl_task();
    circle_task();
    light_repeat_task();
    housekeeping_task_user();
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    track_circle(keycode, record);
    track_light_repeat(keycode, record);
    if (record->event.pressed && keycode != confirm_keycode) {
        confirm_keycode = KC_NO;
    }

    if (!process_record_user(keycode, record)) {
        return false;
    }

    switch (keycode) {
        case R4_PWR_ON:
            if (record->event.pressed) reform4_power_on();
            return false;
        case R4_PWR_OFF:
            if (record->event.pressed) reform4_power_off();
            return false;
        case R4_WAKE:
            if (record->event.pressed) {
                sysctl_wake();
                reform4_oled_message("Wake sent.", NULL, 1500);
            }
            return false;
        case R4_STATUS:
            if (record->event.pressed) {
                sysctl_request_status();
                reform4_oled_show(R4_OLED_STATUS, 5000);
            }
            return false;
        case R4_BATT:
            if (record->event.pressed) {
                sysctl_request_battery(false);
                reform4_oled_show(R4_OLED_BATTERY, 10000);
            }
            return false;
        case R4_LOGO:
            if (record->event.pressed) reform4_oled_show(R4_OLED_SPLASH, 1500);
            return false;
        case R4_USB_RST:
            if (record->event.pressed) {
                restart_usb_driver(&USBD1);
                reform4_oled_message("USB reconnected.", NULL, 1500);
            }
            return false;
        case R4_REBOOT:
            if (record->event.pressed && confirmed(keycode, "R", "reset the keyboard.")) {
                flush_message("Resetting keyboard.", NULL);
                soft_reset_keyboard();
            }
            return false;
        case R4_BOOT:
            if (record->event.pressed && confirmed(keycode, "X", "enter the bootloader.")) {
                flush_message("Entered firmware", "update mode.");
                reset_keyboard();
            }
            return false;
        case RGB_MATRIX_KEYCODE_RANGE:
            // The Circle legend already shows the values while held.
            if (record->event.pressed && !layer_state_is(R4_LAYER_CIRCLE)) {
                reform4_oled_show(R4_OLED_RGB, 2000);
            }
            return true;
    }

    // Typing while the laptop is off: show how to power it up, like the
    // MNT firmware's hint page.
    if (record->event.pressed && sysctl_power_state() == SYSCTL_POWER_OFF && (IS_BASIC_KEYCODE(keycode) || IS_MODIFIER_KEYCODE(keycode))) {
        reform4_oled_message("Computer is off.", "Hold CIRCLE: power up", 3000);
    }

    return true;
}
