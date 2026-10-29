// Copyright 2026 Andreas Hirschauer
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// Columns are driven high and rows read with pull-downs; 1us matches the
// settle time used by the MNT firmware.
#define MATRIX_IO_DELAY 1

#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 200U

// System controller UART (UART1, GP4 TX / GP5 RX).
#define UART_DRIVER SIOD1
#define UART_TX_PIN GP4
#define UART_RX_PIN GP5
// 57600 for the classic Reform LPC; Reform Next and Pocket use 115200.
#ifndef REFORM4_SYSCTL_BAUD
#    define REFORM4_SYSCTL_BAUD 57600
#endif

// Holding Circle this long (with no other key) sends power on.
#ifndef REFORM4_CIRCLE_POWER_ON_MS
#    define REFORM4_CIRCLE_POWER_ON_MS 2000
#endif

// rgb_driver.c drives the WS2812 chain for RGB Matrix (driver "custom"),
// so the WS2812 driver does not derive its LED count by itself.
#define WS2812_LED_COUNT RGB_MATRIX_LED_COUNT

// SSD1306 128x32 on I2C0 (GP0 SDA / GP1 SCL).
#define I2C_DRIVER I2CD0
#define I2C1_SDA_PIN GP0
#define I2C1_SCL_PIN GP1
#define OLED_DISPLAY_128X32
#define OLED_FONT_H "reform4_font.c"
#define OLED_FONT_END 255
#define OLED_TIMEOUT 10000
#define RGB_MATRIX_MODE_NAME_ENABLE
