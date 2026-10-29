// Copyright 2026 Andreas Hirschauer
// SPDX-License-Identifier: GPL-2.0-or-later

// RGB Matrix driver on top of the WS2812 driver that only sends a frame when
// a colour changed. RGB Matrix flushes every ~16 ms whether or not anything
// changed, and on this board a steady stream of LED frames disturbs USB:
// hosts reset the keyboard, keys drop and the backlight flickers, at any
// brightness above zero. A static effect now goes out once, like with
// RGBLight. Animated and reactive effects update every frame and trigger the
// same problem, so keyboard.json only enables static ones.

#include "gpio.h"
#include "rgb_matrix.h"
#include "ws2812.h"

// The WS2812 driver sets up the data pin with fast slew. Use the RP2040
// default MNT's firmware runs with (slow slew, 4 mA) to keep the LED data
// edges soft.
#ifndef REFORM4_WS2812_PAD_MODE
#    define REFORM4_WS2812_PAD_MODE (PAL_RP_GPIO_OE | PAL_RP_PAD_DRIVE4 | PAL_MODE_ALTERNATE_PIO0)
#endif

static uint8_t shadow[RGB_MATRIX_LED_COUNT][3];
static bool    dirty = true;

static void init(void) {
    ws2812_init();
    palSetLineMode(WS2812_DI_PIN, REFORM4_WS2812_PAD_MODE);
    dirty = true;
}

static void set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t *led = shadow[index];
    if (led[0] == r && led[1] == g && led[2] == b) {
        return;
    }
    led[0] = r;
    led[1] = g;
    led[2] = b;
    ws2812_set_color(index, r, g, b);
    dirty = true;
}

static void set_color_all(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        set_color(i, r, g, b);
    }
}

static void flush(void) {
    if (!dirty) {
        return;
    }
    ws2812_flush();
    dirty = false;
}

const rgb_matrix_driver_t rgb_matrix_driver = {
    .init          = init,
    .set_color     = set_color,
    .set_color_all = set_color_all,
    .flush         = flush,
};
