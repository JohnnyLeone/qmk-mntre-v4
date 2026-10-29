// Copyright 2026 Andreas Hirschauer
// SPDX-License-Identifier: GPL-2.0-or-later

// 21x4 character pages on the SSD1306. The screen stays dark unless a page
// is active, like the MNT firmware, which only lights it for the menu.

#include QMK_KEYBOARD_H

#include <stdio.h>
#include <string.h>

#include "sysctl.h"
#include "version.h"

#define OLED_COLS 21
#define BATTERY_REFRESH_MS 1000
#define LEGEND_PAGE_MS 2500

static reform4_oled_page_t page = R4_OLED_NONE;
static uint32_t            page_until;
static char                message[2][OLED_COLS + 1];
static uint8_t             power_on_percent;
static uint32_t            battery_refresh_at;

// What is on screen, to clear exactly once per page change.
typedef enum {
    SHOWN_NONE,
    SHOWN_PAGE,
    SHOWN_CIRCLE_MENU,
} shown_t;
static shown_t             shown = SHOWN_NONE;
static reform4_oled_page_t shown_page;
static uint32_t            shown_since;

void reform4_oled_show(reform4_oled_page_t new_page, uint32_t duration_ms) {
    page       = new_page;
    page_until = timer_read32() + duration_ms;
    // Redraw from scratch, also when the same page is requested again.
    shown = SHOWN_NONE;
    if (new_page == R4_OLED_BATTERY) {
        battery_refresh_at = timer_read32() + BATTERY_REFRESH_MS;
    }
}

void reform4_oled_message(const char *line1, const char *line2, uint32_t duration_ms) {
    strlcpy(message[0], line1 ? line1 : "", sizeof(message[0]));
    strlcpy(message[1], line2 ? line2 : "", sizeof(message[1]));
    reform4_oled_show(R4_OLED_MESSAGE, duration_ms);
}

void reform4_oled_power_on_progress(uint8_t percent) {
    power_on_percent = percent;
}

oled_rotation_t oled_init_kb(oled_rotation_t rotation) {
    return oled_init_user(rotation);
}

// Write one full row, padded with spaces so stale characters disappear and
// unchanged rows do not mark the display dirty.
static void write_row(uint8_t row, const char *text) {
    char   buf[OLED_COLS + 1];
    size_t len = strlen(text);
    if (len > OLED_COLS) {
        len = OLED_COLS;
    }
    memcpy(buf, text, len);
    memset(buf + len, ' ', OLED_COLS - len);
    buf[OLED_COLS] = '\0';
    oled_set_cursor(0, row);
    oled_write(buf, false);
}

static void render_splash(void) {
    // MNT logo, three rows of twelve glyphs starting at 160.
    for (uint8_t y = 0; y < 3; y++) {
        oled_set_cursor(4, y + 1);
        for (uint8_t x = 0; x < 12; x++) {
            oled_write_char((char)((5 + y) * 32 + x), false);
        }
    }
}

static void render_message(void) {
    write_row(1, message[0]);
    write_row(2, message[1]);
}

static void render_rgb_values(uint8_t row) {
    char buf[OLED_COLS + 1];
    if (!rgb_matrix_is_enabled()) {
        write_row(row, "Backlight off");
        return;
    }
    snprintf(buf, sizeof(buf), "H%3u S%3u V%3u  #%2u", rgb_matrix_get_hue(), rgb_matrix_get_sat(), rgb_matrix_get_val(), rgb_matrix_get_mode());
    write_row(row, buf);
}

static void render_rgb(void) {
    write_row(0, "Backlight");
    if (rgb_matrix_is_enabled()) {
        char name[OLED_COLS + 1];
        strlcpy(name, rgb_matrix_get_mode_name(rgb_matrix_get_mode()), sizeof(name));
        for (char *c = name; *c; c++) {
            if (*c == '_') *c = ' ';
        }
        write_row(1, name);
    } else {
        write_row(1, "");
    }
    render_rgb_values(2);
}

static void render_power_on_bar(uint8_t row) {
    char          bar[OLED_COLS + 1] = "Power on ";
    const uint8_t start              = (uint8_t)strlen(bar);
    const uint8_t inner              = OLED_COLS - start;
    const uint8_t filled             = (uint8_t)((uint16_t)power_on_percent * inner / 100);
    for (uint8_t i = 0; i < inner; i++) {
        bar[start + i] = i < filled ? '=' : '.';
    }
    bar[OLED_COLS] = '\0';
    write_row(row, bar);
}

// Legend for the Circle layer, mirroring the MNT firmware's Circle menu.
// Three rows of text alternate between two pages while the layer is held.
static const char *const legend[2][3] = {
    {"1 Power on   0 Off", "B Battery  S Status", "SPC Wake     L Logo"},
    {"F1-6 Bright/Hue/Sat", "F7/8 Mode  F9 On/Off", "R Reset U USB X Boot"},
};

static void render_circle_menu(void) {
    const uint8_t legend_page = (timer_elapsed32(shown_since) / LEGEND_PAGE_MS) % 2;
    for (uint8_t row = 0; row < 3; row++) {
        write_row(row, legend[legend_page][row]);
    }
    if (power_on_percent > 0) {
        render_power_on_bar(3);
    } else {
        render_rgb_values(3);
    }
}

static void render_no_reply(void) {
    write_row(0, "");
    write_row(1, "No reply from the");
    write_row(2, "system controller.");
    write_row(3, "");
}

// Battery icon glyph pair (128 + 0/2/4/6/8) for a cell voltage, same
// thresholds as the MNT firmware.
static uint8_t battery_icon(uint8_t dv) {
    if (dv >= 33) return 128 + 8;
    if (dv >= 31) return 128 + 6;
    if (dv >= 30) return 128 + 4;
    if (dv >= 29) return 128 + 2;
    return 128;
}

static void render_battery(void) {
    const sysctl_battery_t *bat = sysctl_battery();

    if (sysctl_reply_state() != SYSCTL_REPLY_PENDING && timer_expired32(timer_read32(), battery_refresh_at)) {
        battery_refresh_at = timer_read32() + BATTERY_REFRESH_MS;
        sysctl_request_battery(true);
    }

    if (!bat->valid) {
        if (sysctl_reply_state() == SYSCTL_REPLY_PENDING) {
            write_row(1, "Reading battery...");
        } else {
            render_no_reply();
        }
        return;
    }

    const char *power;
    switch (sysctl_power_state()) {
        case SYSCTL_POWER_ON:
            power = "  On";
            break;
        case SYSCTL_POWER_OFF:
            power = " Off";
            break;
        default:
            power = "   ?";
            break;
    }

    const int16_t  ma     = bat->current_ma;
    const uint16_t abs_ma = ma < 0 ? (uint16_t)-ma : (uint16_t)ma;
    char           tail[4][10];
    snprintf(tail[0], sizeof(tail[0]), "   %s", bat->gauge);
    snprintf(tail[1], sizeof(tail[1]), "   %s", power);
    snprintf(tail[2], sizeof(tail[2]), "%c%u.%03uA", ma < 0 ? '-' : ' ', abs_ma / 1000, abs_ma % 1000);
    snprintf(tail[3], sizeof(tail[3]), " %2u.%02uV", bat->voltage_mv / 1000, (bat->voltage_mv % 1000) / 10);

    // Two columns of four cells, then the pack summary on the right.
    for (uint8_t row = 0; row < 4; row++) {
        // Cells are parsed from two digits; the clamp tells the compiler too.
        const uint8_t l = MIN(bat->cell_dv[row], 99), r = MIN(bat->cell_dv[row + 4], 99);
        char          buf[OLED_COLS + 1];
        snprintf(buf, sizeof(buf), "%c%c %u.%u  %c%c %u.%u%.7s", battery_icon(l), battery_icon(l) + 1, l / 10, l % 10, battery_icon(r), battery_icon(r) + 1, r / 10, r % 10, tail[row]);
        write_row(row, buf);
    }
}

static void render_status(void) {
    switch (sysctl_reply_state()) {
        case SYSCTL_REPLY_PENDING:
            write_row(1, "Asking system ctl...");
            return;
        case SYSCTL_REPLY_DONE:
            break;
        default:
            render_no_reply();
            return;
    }

    // Wrap the reply over three rows, firmware version below.
    const char *text = sysctl_reply_text();
    for (uint8_t row = 0; row < 3; row++) {
        char buf[OLED_COLS + 1] = "";
        if (strlen(text) > row * OLED_COLS) {
            strlcpy(buf, text + row * OLED_COLS, sizeof(buf));
        }
        write_row(row, buf);
    }
    write_row(3, "QMK " QMK_VERSION);
}

static void render_page(void) {
    switch (page) {
        case R4_OLED_SPLASH:
            render_splash();
            break;
        case R4_OLED_MESSAGE:
            render_message();
            break;
        case R4_OLED_RGB:
            render_rgb();
            break;
        case R4_OLED_BATTERY:
            render_battery();
            break;
        case R4_OLED_STATUS:
            render_status();
            break;
        case R4_OLED_NONE:
            break;
    }
}

bool oled_task_kb(void) {
    if (!oled_task_user()) {
        return false;
    }

    if (page != R4_OLED_NONE && timer_expired32(timer_read32(), page_until)) {
        page = R4_OLED_NONE;
    }

    // Priority: the power-on countdown (shown in the Circle legend), a page
    // triggered by an action, then the Circle legend while the layer is held.
    shown_t want = SHOWN_NONE;
    if (power_on_percent > 0) {
        want = SHOWN_CIRCLE_MENU;
    } else if (page != R4_OLED_NONE) {
        want = SHOWN_PAGE;
    } else if (layer_state_is(R4_LAYER_CIRCLE)) {
        want = SHOWN_CIRCLE_MENU;
    }

    if (want != shown || (want == SHOWN_PAGE && page != shown_page)) {
        oled_clear();
        shown       = want;
        shown_page  = page;
        shown_since = timer_read32();
        if (want == SHOWN_NONE) {
            oled_render_dirty(true);
            oled_off();
            return false;
        }
    }

    switch (want) {
        case SHOWN_PAGE:
            render_page();
            break;
        case SHOWN_CIRCLE_MENU:
            render_circle_menu();
            break;
        case SHOWN_NONE:
            break;
    }
    return false;
}
