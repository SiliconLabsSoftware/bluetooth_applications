/***************************************************************************//**
 * @file app_thermostat_display.c
 * @brief Display application code
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided \'as-is\', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/
#include <string.h>
#include <stdio.h>
#include "app_timer.h"
#include "sl_i2cspm_instances.h"
#include "micro_oled_ssd1306.h"
#include "glib.h"
#include "app_display.h"
#include "app_thermostat.h"

#define DISPLAY_INTERVAL_MS  500

static const unsigned char silicon_labs_logo_64x23[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x80, 0xff, 0x1f, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xf0,
  0xff, 0x7f, 0x00, 0x3e, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x7f, 0x00, 0x7f,
  0x00, 0x00, 0x00, 0xff, 0xff, 0x3f, 0x80, 0x7f, 0x00, 0x06, 0x80, 0xff,
  0xff, 0xff, 0xe0, 0xff, 0x80, 0x03, 0xc0, 0xff, 0x01, 0xf8, 0xff, 0x7f,
  0xe0, 0x01, 0xc0, 0x03, 0x00, 0x00, 0xfe, 0x7f, 0xf0, 0x01, 0x00, 0x01,
  0x00, 0x00, 0xf8, 0x3f, 0xf8, 0x03, 0x00, 0xf0, 0x07, 0x00, 0xe0, 0x3f,
  0xfc, 0x03, 0x00, 0x00, 0x20, 0x00, 0xe0, 0x1f, 0xfe, 0x0f, 0x00, 0x00,
  0xc0, 0x00, 0xe0, 0x07, 0xfe, 0xff, 0x00, 0x00, 0xfc, 0x00, 0xe0, 0x03,
  0xff, 0xc3, 0xff, 0xff, 0xff, 0x00, 0xf0, 0x00, 0x7e, 0x00, 0xfe, 0xff,
  0x7f, 0x00, 0x38, 0x00, 0x3e, 0x00, 0xff, 0xff, 0x3f, 0x00, 0x0c, 0x00,
  0x1c, 0x80, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x18, 0x00, 0xff, 0xff,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x1f, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

static glib_context_t glib_context;
static app_timer_t display_timer;

static void oled_update(app_thermostat_t app_data);
static void display_timer_cb(app_timer_t *timer, void *data);

// -----------------------------------------------------------------------------
// Public function definitions

/***************************************************************************//**
 * Initialize the OLED display.
 ******************************************************************************/
sl_status_t oled_init(void)
{
  if (SL_STATUS_OK != ssd1306_init(sl_i2cspm_qwiic)) {
    return SL_STATUS_FAIL;
  }

  glib_init(&glib_context);

  // Fill lcd with background color
  glib_clear(&glib_context);

  glib_draw_xbitmap(&glib_context,
                    0, 12, silicon_labs_logo_64x23,
                    64, 23, GLIB_WHITE);
  glib_update_display();
  sl_sleeptimer_delay_millisecond(1000);

  // Fill lcd with background color
  glib_clear(&glib_context);

  glib_update_display();

  // Start timer used for display operations
  app_timer_start(&display_timer,
                  DISPLAY_INTERVAL_MS,
                  display_timer_cb,
                  NULL,
                  true);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Show temperature, humidity, setpoint and hysteresis on OLED display
 ******************************************************************************/
static void oled_update(app_thermostat_t app_data)
{
  char number_str[15];

  // Fill lcd with background color
  glib_clear(&glib_context);

  // Use Narrow font
  glib_draw_string(&glib_context, "THERMOSTAT", 0, 0);
  glib_draw_line(&glib_context, 0, 8, 95, 8, GLIB_WHITE);

  glib_draw_line(&glib_context, 0, 29, 95, 29, GLIB_WHITE);

  snprintf(number_str, sizeof(number_str), "%5.1f%cC",
           (float)app_data.data.temperature / 10, 0xf8);
  glib_draw_string(&glib_context, number_str, 10, 11);

  snprintf(number_str, sizeof(number_str), "%5.1f %c",
           (float)app_data.data.humidity / 10, '%');
  glib_draw_string(&glib_context, number_str, 10, 21);

  snprintf(number_str, sizeof(number_str), "%.1f%cC",
           (float)app_data.config.setpoint / 10, 0xf8);
  glib_draw_string(&glib_context, number_str, 10, 32);

  snprintf(number_str, sizeof(number_str), "(%.1f%cC)",
           (float)app_data.config.hysteresis / 10, 0xf8);
  glib_draw_string(&glib_context, number_str, 10, 41);

  glib_update_display();
}

/***************************************************************************//**
 * Callback on timer period.
 ******************************************************************************/
static void display_timer_cb(app_timer_t *timer, void *data)
{
  (void)data;
  (void)timer;

  app_thermostat_t measurement_data = thermostat_get_app_data();
  oled_update(measurement_data);
}
