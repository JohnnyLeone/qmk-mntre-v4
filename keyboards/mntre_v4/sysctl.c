// Copyright 2026 Andreas Hirschauer
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sysctl.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "timer.h"
#include "uart.h"

#define SYSCTL_REPLY_TIMEOUT_MS 250
#define SYSCTL_POWER_ON_REPEAT_MS 50
#define SYSCTL_REPLY_MAX 128

typedef enum {
    EXPECT_NONE,
    EXPECT_BATTERY,
    EXPECT_TEXT,
} expect_t;

static char                 reply[SYSCTL_REPLY_MAX];
static uint8_t              reply_len;
static expect_t             expect;
static sysctl_reply_state_t reply_state;
static uint32_t             reply_deadline;

static bool     power_on_repeat_pending;
static uint32_t power_on_repeat_at;

static sysctl_power_t   power_state;
static sysctl_battery_t battery;

static void send(const char *cmd) {
    uart_transmit((const uint8_t *)cmd, (uint16_t)strlen(cmd));
}

static void request(const char *cmd, expect_t what) {
    // Drop stale bytes (e.g. the '\n' after a previous reply).
    while (uart_available()) {
        uart_read();
    }
    reply_len      = 0;
    reply[0]       = '\0';
    expect         = what;
    reply_state    = SYSCTL_REPLY_PENDING;
    reply_deadline = timer_read32() + SYSCTL_REPLY_TIMEOUT_MS;
    send(cmd);
}

// Fixed-width field from the battery reply, as an integer.
static long field(const char *src, uint8_t len) {
    char buf[8];
    memcpy(buf, src, len);
    buf[len] = '\0';
    return strtol(buf, NULL, 10);
}

// LPC reply to "c\r", as parsed by the MNT firmware:
//   32 32 32 32 32 32 32 32 mA 0256mV26143 ???% P1
//   |  |  ... 8 cells, 0.1V  |  |      |     |    `- 45: system power
//                            |  26     33    39
//                            |  current     gauge
//                            `- the current may carry a minus sign
static bool parse_battery(void) {
    if (reply_len < 46) {
        return false;
    }

    for (uint8_t i = 0; i < 8; i++) {
        const char *c = &reply[i * 3];
        if (!isdigit((unsigned char)c[0]) || !isdigit((unsigned char)c[1])) {
            return false;
        }
        battery.cell_dv[i] = (uint8_t)((c[0] - '0') * 10 + (c[1] - '0'));
    }

    battery.current_ma = (int16_t)field(&reply[26], 5);
    battery.voltage_mv = (uint16_t)field(&reply[33], 5);
    memcpy(battery.gauge, &reply[39], 4);
    battery.gauge[4] = '\0';

    if (reply[45] == '1') {
        power_state = SYSCTL_POWER_ON;
    } else if (reply[45] == '0') {
        power_state = SYSCTL_POWER_OFF;
    }

    battery.valid = true;
    return true;
}

static void reply_complete(void) {
    reply[reply_len] = '\0';
    if (expect == EXPECT_BATTERY && !parse_battery()) {
        battery.valid = false;
    }
    expect      = EXPECT_NONE;
    reply_state = SYSCTL_REPLY_DONE;
}

void sysctl_init(void) {
    uart_init(REFORM4_SYSCTL_BAUD);
}

void sysctl_task(void) {
    while (uart_available()) {
        const char c = (char)uart_read();

        if (expect == EXPECT_NONE) {
            continue;
        }
        if (c == '\r') {
            if (reply_len > 0) {
                reply_complete();
            }
        } else if (c == '\n') {
            // The battery reply is a single line; for text replies keep
            // all lines, separated by spaces.
            if (expect == EXPECT_BATTERY) {
                reply_len = 0;
            } else if (reply_len > 0 && reply_len < SYSCTL_REPLY_MAX - 1) {
                reply[reply_len++] = ' ';
            }
        } else if (reply_len < SYSCTL_REPLY_MAX - 1) {
            reply[reply_len++] = c;
        }
    }

    const uint32_t now = timer_read32();

    if (reply_state == SYSCTL_REPLY_PENDING && timer_expired32(now, reply_deadline)) {
        expect      = EXPECT_NONE;
        reply_state = SYSCTL_REPLY_TIMEOUT;
    }

    if (power_on_repeat_pending && timer_expired32(now, power_on_repeat_at)) {
        power_on_repeat_pending = false;
        send("1p\r");
    }
}

void sysctl_power_on(void) {
    // There may be noise left in the sysctl UART since turning off, so the
    // command is sent twice, like the MNT firmware does.
    send("1p\r");
    power_on_repeat_pending = true;
    power_on_repeat_at      = timer_read32() + SYSCTL_POWER_ON_REPEAT_MS;
    power_state             = SYSCTL_POWER_ON;
}

void sysctl_power_off(void) {
    power_on_repeat_pending = false;
    send("0p\r");
    power_state = SYSCTL_POWER_OFF;
}

void sysctl_wake(void) {
    send("1w\r");
}

void sysctl_request_battery(bool keep_last) {
    if (!keep_last) {
        battery.valid = false;
    }
    request("c\r", EXPECT_BATTERY);
}

void sysctl_request_status(void) {
    request("s\r", EXPECT_TEXT);
}

sysctl_power_t sysctl_power_state(void) {
    return power_state;
}

sysctl_reply_state_t sysctl_reply_state(void) {
    return reply_state;
}

const sysctl_battery_t *sysctl_battery(void) {
    return &battery;
}

const char *sysctl_reply_text(void) {
    return reply;
}
