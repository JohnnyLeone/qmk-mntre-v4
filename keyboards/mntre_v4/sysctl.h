// Copyright 2026 Andreas Hirschauer
// SPDX-License-Identifier: GPL-2.0-or-later

// Non-blocking client for the Reform system controller (LPC/sysctl) that
// listens on the keyboard UART. Commands are ASCII lines ending in '\r'.

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SYSCTL_POWER_UNKNOWN,
    SYSCTL_POWER_OFF,
    SYSCTL_POWER_ON,
} sysctl_power_t;

typedef enum {
    SYSCTL_REPLY_IDLE,
    SYSCTL_REPLY_PENDING,
    SYSCTL_REPLY_DONE,
    SYSCTL_REPLY_TIMEOUT,
} sysctl_reply_state_t;

typedef struct {
    bool     valid;
    uint8_t  cell_dv[8]; // cell voltages in 0.1 V
    int16_t  current_ma; // negative while charging
    uint16_t voltage_mv;
    char     gauge[5]; // state of charge as sent, e.g. " 87%" or "???%"
} sysctl_battery_t;

void sysctl_init(void);
void sysctl_task(void);

void sysctl_power_on(void);
void sysctl_power_off(void);
void sysctl_wake(void);

// keep_last: keep showing the previous reading until the new one arrives.
void sysctl_request_battery(bool keep_last);
void sysctl_request_status(void);

sysctl_power_t          sysctl_power_state(void);
sysctl_reply_state_t    sysctl_reply_state(void);
const sysctl_battery_t *sysctl_battery(void);
const char             *sysctl_reply_text(void);
