// Copyright 2026 Andreas Hirschauer
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"

// Layer indices shared by the keymaps and the OLED legend pages.
// Custom keycodes (R4_PWR_ON, ...) are generated from keyboard.json.
enum reform4_layers {
    R4_LAYER_BASE,
    R4_LAYER_HYPER,
    R4_LAYER_CIRCLE,
};

// Circle key: tap for F24, hold for the lighting/menu layer, long-hold
// (REFORM4_CIRCLE_POWER_ON_MS) to power on the laptop.
#define R4_CIRC LT(R4_LAYER_CIRCLE, KC_F24)
#define R4_HYPR MO(R4_LAYER_HYPER)

typedef enum {
    R4_OLED_NONE,
    R4_OLED_SPLASH,
    R4_OLED_MESSAGE,
    R4_OLED_RGB,
    R4_OLED_BATTERY,
    R4_OLED_STATUS,
} reform4_oled_page_t;

void reform4_oled_show(reform4_oled_page_t page, uint32_t duration_ms);
void reform4_oled_message(const char *line1, const char *line2, uint32_t duration_ms);
void reform4_oled_power_on_progress(uint8_t percent);
