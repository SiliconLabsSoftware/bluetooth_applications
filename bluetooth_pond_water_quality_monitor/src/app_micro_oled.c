/***************************************************************************//**
 * @file app_micro_oled.c
 * @brief Main application logic for displaying on OLED.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 ********************************************************************************
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
 * This code has been minimally tested to ensure that it builds and is suitable
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/
#include "micro_oled_ssd1306.h"
#include "sl_i2cspm_instances.h"
#include "app_micro_oled.h"
#include "glib_font.h"
#include "glib.h"

#define SILABS_LOGO_WIDTH         (64)
#define SILABS_LOGO_HEIGHT        (23)
#define SILABS_X_POSITION         (0)
#define SILABS_Y_POSITION         (0)
#define SILABS_STR_X_POSITION     (2)
#define SILABS_STR_Y_POSITION     (35)

#define FIRST_LINE_X0_POSITION    (0)
#define FIRST_LINE_Y0_POSITION    (11)
#define FIRST_LINE_X1_POSITION    (63)
#define FIRST_LINE_Y1_POSITION    (11)

#define SECOND_LINE_X0_POSITION   (0)
#define SECOND_LINE_Y0_POSITION   (33)
#define SECOND_LINE_X1_POSITION   (63)
#define SECOND_LINE_Y1_POSITION   (33)

#define RECTANGLE_X_POSITION      (0)
#define RECTANGLE_Y_POSITION      (35)
#define RECTANGLE_MID_X_POSITION  (31)
#define RECTANGLE_WIDTH           (63)
#define RECTANGLE_HEIGHT          (10)

#define SHORT_STRING_X_POSITION   (10)
#define SHORT_STRING_Y_POSITION   (20)
#define LONG_STRING_X_POSITION    (8)
#define LONG_STRING_Y_POSITION    (20)
#define STRING_LEN                (20)

#define draw_vline(g, x, y, h, color) \
  glib_draw_line(g, x, y, x, y + h - 1, color)

static glib_context_t glib_context;
static mikroe_i2c_handle_t app_i2c_instance = NULL;
static glib_status_t glib_fill_rect_reverse(glib_context_t *g_context,
                                            int16_t x, int16_t y,
                                            int16_t w, int16_t h,
                                            uint16_t color);

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

void app_sparkfun_micro_oled_init(void)
{
  app_i2c_instance = sl_i2cspm_qwiic;

  // Initialize the display
  ssd1306_init(app_i2c_instance);
  glib_init(&glib_context);

  // Fill lcd with background color
  glib_clear(&glib_context);

  glib_draw_xbitmap(&glib_context,
                    SILABS_X_POSITION,
                    SILABS_Y_POSITION,
                    silicon_labs_logo_64x23,
                    SILABS_LOGO_WIDTH,
                    SILABS_LOGO_HEIGHT,
                    GLIB_WHITE);

  glib_draw_string(&glib_context,
                   "  SiLabs",
                   SILABS_STR_X_POSITION,
                   SILABS_STR_Y_POSITION);
  glib_update_display();
}

void app_sparkfun_micro_oled_process(float pH_display_value)
{
  if ((pH_display_value > 14) || (pH_display_value < 0)) {
    return;
  }

  char result[STRING_LEN] = { 0 };
  float percentage = pH_display_value / MAX_PH_VALUE;
  int len = snprintf(NULL, 0, "  %.2f", pH_display_value);

  glib_clear(&glib_context);
  if (pH_display_value >= 10) {
    snprintf(result, len + 1, " %.2f", pH_display_value);
    glib_draw_string(&glib_context,
                     result,
                     SHORT_STRING_X_POSITION,
                     SHORT_STRING_Y_POSITION);
  } else {
    snprintf(result, len + 1, "  %.2f", pH_display_value);
    glib_draw_string(&glib_context,
                     result,
                     LONG_STRING_X_POSITION,
                     LONG_STRING_Y_POSITION);
  }

  glib_draw_string(&glib_context, " pH VALUE", 2, 2);

  glib_draw_line(&glib_context,
                 FIRST_LINE_X0_POSITION,
                 FIRST_LINE_Y0_POSITION,
                 FIRST_LINE_X1_POSITION,
                 FIRST_LINE_Y1_POSITION,
                 GLIB_WHITE);

  glib_draw_line(&glib_context,
                 SECOND_LINE_X0_POSITION,
                 SECOND_LINE_Y0_POSITION,
                 SECOND_LINE_X1_POSITION,
                 SECOND_LINE_Y1_POSITION,
                 GLIB_WHITE);

  glib_set_font(&glib_context, NULL);

  glib_draw_rect(&glib_context,
                 RECTANGLE_X_POSITION,
                 RECTANGLE_Y_POSITION,
                 RECTANGLE_WIDTH,
                 RECTANGLE_HEIGHT,
                 GLIB_WHITE);

  if (pH_display_value > NEUTRAL_PH_VALUE) {
    percentage = (pH_display_value - NEUTRAL_PH_VALUE) / NEUTRAL_PH_VALUE;
    glib_fill_rect(&glib_context,
                   RECTANGLE_MID_X_POSITION,
                   RECTANGLE_Y_POSITION,
                   (RECTANGLE_MID_X_POSITION * percentage),
                   RECTANGLE_HEIGHT, GLIB_WHITE);
  } else if (pH_display_value <= NEUTRAL_PH_VALUE) {
    percentage = (NEUTRAL_PH_VALUE - pH_display_value) / NEUTRAL_PH_VALUE;
    glib_fill_rect_reverse(&glib_context,
                           RECTANGLE_MID_X_POSITION,
                           RECTANGLE_Y_POSITION,
                           (RECTANGLE_MID_X_POSITION * percentage),
                           RECTANGLE_HEIGHT,
                           GLIB_WHITE);
  }

  if (glib_update_display() != GLIB_OK) {
    return;
  }
}

static glib_status_t glib_fill_rect_reverse(glib_context_t *g_context,
                                            int16_t x, int16_t y,
                                            int16_t w, int16_t h,
                                            uint16_t color)
{
  glib_status_t status = GLIB_OK;

  for (int16_t i = x; i >= x - w; i--) {
    status |= draw_vline(g_context, i, y, h, color);
  }
  return status;
}
