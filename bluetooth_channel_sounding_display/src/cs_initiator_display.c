/***************************************************************************//**
 * @file cs_initiator_display.c
 * @brief CS Initiator display
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
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

#include "cs_initiator_display.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "app_config.h"
#include "trace.h"

#define DISTANCE_DISPLAY_Y_OFFSET         (20)
#define UNIT_DISPLAY_Y_OFFSET             (24)

#define DOT_Y_OFFSET                      (34 + DISTANCE_DISPLAY_Y_OFFSET)
#define DOT_X_OFFSET                      (2)
#define DOT_X_SIZE                        (6)
#define DOT_Y_SIZE                        (6)

#define UNIT_X_OFFSET                     (4)

#define DISTANCE_INTEGER_3DIGITS_X_OFFSET (18)
#define UNIT_3DIGITS_X_OFFSET             (DISTANCE_INTEGER_3DIGITS_X_OFFSET \
                                           + 72 + UNIT_X_OFFSET)

#define DISTANCE_INTEGER_2DIGITS_X_OFFSET (13)
#define DOT_2DIGITS_X_OFFSET              (DISTANCE_INTEGER_2DIGITS_X_OFFSET \
                                           + 48 + DOT_X_OFFSET)
#define DISTANCE_DECIMAL_2DIGITS_X_OFFSET (DOT_2DIGITS_X_OFFSET + DOT_X_SIZE \
                                           + DOT_X_OFFSET)
#define UNIT_2DIGITS_X_OFFSET             (DISTANCE_DECIMAL_2DIGITS_X_OFFSET \
                                           + 24 + UNIT_X_OFFSET)

#define DISTANCE_INTEGER_1DIGIT_X_OFFSET  (25)
#define DOT_1DIGIT_X_OFFSET               (DISTANCE_INTEGER_1DIGIT_X_OFFSET \
                                           + 24 + DOT_X_OFFSET)
#define DISTANCE_DECIMAL_1DIGIT_X_OFFSET  (DOT_1DIGIT_X_OFFSET + DOT_X_SIZE \
                                           + DOT_X_OFFSET)
#define UNIT_1DIGIT_X_OFFSET              (DISTANCE_DECIMAL_1DIGIT_X_OFFSET \
                                           + 24 + UNIT_X_OFFSET)

#define PROGRESS_BAR_EDGE_SIZE            (3)
#define PROGRESS_BAR_SPACE_OFFSET         (1)
#define PROGRESS_BAR_HEIGHT               (20)
#define PROGRESS_BAR_BOTTOM_OFFSET        (5)
#define PROGRESS_BAR_MAX                  (128 - (2 * PROGRESS_BAR_EDGE_SIZE) \
                                           - (2 * PROGRESS_BAR_SPACE_OFFSET))

static GLIB_Context_t glib_context;
static cs_initiator_display_content_t lcd_content;
extern const GLIB_Font_t GLIB_FontNumber24x40;

#if (MEASUREMENT_UNIT_METERS == CS_MEASUREMENT_UNIT) // Meters
extern const GLIB_Font_t GLIB_Font16x16;
#elif (MEASUREMENT_UNIT_FEET == CS_MEASUREMENT_UNIT) // Feet

#define METER_TO_FEET_CONSTANT_VALUE  (3.28084F)
extern const GLIB_Font_t GLIB_Font8x16;
#endif

static void cs_initiator_display_measurement_unit(uint8_t x_offset,
                                                  uint8_t y_offset);
static void cs_initiator_display_dot_character(uint8_t x_offset,
                                               uint8_t y_offset);
static void cs_initiator_display_clear_distance_area(void);

/*******************************************************************************
 * Initialize the display.
 ******************************************************************************/
sl_status_t cs_initiator_display_init(void)
{
  memset(&lcd_content, 0, sizeof(cs_initiator_display_content_t));

  EMSTATUS status = DMD_init(0);
  if (status != DMD_OK) {
    display_log_error("DMD_init failed! [E:0x%lx]" NL, status);
    return SL_STATUS_INITIALIZATION;
  }

  status = GLIB_contextInit(&glib_context);
  if (status != GLIB_OK) {
    display_log_error("DMD_init failed! [E:0x%lx]" NL, status);
    return SL_STATUS_INITIALIZATION;
  }

  glib_context.backgroundColor = White;
  glib_context.foregroundColor = Black;
  GLIB_clear(&glib_context);

  cs_initiator_display_distance_measurement(0.0);

  // force update LCD content for drawing the first screen
  cs_initiator_display_update();
  return SL_STATUS_OK;
}

/*******************************************************************************
 * CS initiator display measured distance.
 ******************************************************************************/
void cs_initiator_display_distance_measurement(float measured_distance)
{
  if (lcd_content.distance != measured_distance) {
    lcd_content.distance = measured_distance;

    uint8_t x_integer_offset = 0;
    uint8_t x_decimal_offset = 0;
    uint8_t x_dot_offset = 0;
    uint8_t unit_x_offset = 0;
    uint8_t unit_y_offset = DISTANCE_DISPLAY_Y_OFFSET + UNIT_DISPLAY_Y_OFFSET;
    float display_value = 0;
    bool display_decimal_part = false;
    uint8_t y_offset = DISTANCE_DISPLAY_Y_OFFSET;
    uint8_t y_dot_offset = DOT_Y_OFFSET;
    char buffer[20];

// Meters
#if (MEASUREMENT_UNIT_METERS == CS_MEASUREMENT_UNIT)
    display_value = measured_distance;
// Feet
#elif (MEASUREMENT_UNIT_FEET == CS_MEASUREMENT_UNIT)
    display_value = measured_distance * METER_TO_FEET_CONSTANT_VALUE;
#endif

    uint16_t integer_part = truncf(display_value);
    uint16_t decimal_part = (display_value - (float)integer_part) * 10;

    if (integer_part > 99) {
      // 3 digit number, only display integer part example: 100
      x_integer_offset = DISTANCE_INTEGER_3DIGITS_X_OFFSET;
      unit_x_offset = UNIT_3DIGITS_X_OFFSET;
    } else if (integer_part > 9) {
      // 2 digit number, display integer and decimal part example: 10.0
      x_integer_offset = DISTANCE_INTEGER_2DIGITS_X_OFFSET;
      x_decimal_offset = DISTANCE_DECIMAL_2DIGITS_X_OFFSET;
      x_dot_offset = DOT_2DIGITS_X_OFFSET;
      unit_x_offset = UNIT_2DIGITS_X_OFFSET;
      display_decimal_part = true;
    } else {
      // 1 digit number, display integer and decimal part example: 9.9
      x_integer_offset = DISTANCE_INTEGER_1DIGIT_X_OFFSET;
      x_decimal_offset = DISTANCE_DECIMAL_1DIGIT_X_OFFSET;
      x_dot_offset = DOT_1DIGIT_X_OFFSET;
      unit_x_offset = UNIT_1DIGIT_X_OFFSET;
      display_decimal_part = true;
    }

    GLIB_setFont(&glib_context, (GLIB_Font_t *) &GLIB_FontNumber24x40);
    cs_initiator_display_clear_distance_area();

    sprintf(buffer, "%u", integer_part);
    GLIB_drawStringOnLine(&glib_context,
                          (const char *)buffer,
                          0,
                          GLIB_ALIGN_LEFT,
                          x_integer_offset,
                          y_offset,
                          false);

    if (display_decimal_part) {
      sprintf(buffer, "%u", decimal_part);
      GLIB_drawStringOnLine(&glib_context,
                            (const char *)buffer,
                            0,
                            GLIB_ALIGN_LEFT,
                            x_decimal_offset,
                            y_offset,
                            false);

      cs_initiator_display_dot_character(x_dot_offset, y_dot_offset);
    }

    cs_initiator_display_measurement_unit(unit_x_offset, unit_y_offset);
    cs_initiator_display_update();
  }
}

/*******************************************************************************
 * CS initiator display distance progress bar
 ******************************************************************************/
void cs_initiator_display_progress_bar(float measured_distance)
{
  uint8_t max_x_size = glib_context.pDisplayGeometry->xSize;
  uint8_t max_y_size = glib_context.pDisplayGeometry->ySize
                       - PROGRESS_BAR_BOTTOM_OFFSET;
  float distance_percentage = 0;
  uint8_t progress_bar_x = 0;
  uint8_t start_pixel_x = 0;
  uint8_t end_pixel_x = 0;
  uint8_t start_pixel_y = 0;
  uint8_t end_pixel_y = 0;

// Meters
#if (MEASUREMENT_UNIT_METERS == CS_MEASUREMENT_UNIT)
  distance_percentage = measured_distance / (float)MAX_MEASURABLE_VALUE_OF_CS;
// Feet
#elif (MEASUREMENT_UNIT_FEET == CS_MEASUREMENT_UNIT)
  distance_percentage = (measured_distance * METER_TO_FEET_CONSTANT_VALUE)
                        / (float)MAX_MEASURABLE_VALUE_OF_CS;
#endif

  progress_bar_x =
    truncf(distance_percentage
           * (max_x_size - (2 * PROGRESS_BAR_EDGE_SIZE)
              - (2 * PROGRESS_BAR_SPACE_OFFSET))) + 1;
  progress_bar_x =
    (progress_bar_x > PROGRESS_BAR_MAX) ? PROGRESS_BAR_MAX : progress_bar_x;

  log_info("distance_percentage = %02u%%\r\n",
           (uint8_t)(distance_percentage * 100));

  // Draw first progress bar edge
  start_pixel_x = 0;
  end_pixel_x = PROGRESS_BAR_EDGE_SIZE;
  start_pixel_y = max_y_size - PROGRESS_BAR_HEIGHT;
  end_pixel_y = max_y_size;

  for (uint8_t i = start_pixel_x; i < end_pixel_x; i++)
  {
    for (uint8_t j = start_pixel_y; j < end_pixel_y; j++)
    {
      GLIB_drawPixel(&glib_context, i, j);
    }
  }

  // Draw second progress bar edge
  start_pixel_x = max_x_size - PROGRESS_BAR_EDGE_SIZE;
  end_pixel_x = max_x_size;
  start_pixel_y = max_y_size - PROGRESS_BAR_HEIGHT;
  end_pixel_y = max_y_size;

  for (uint8_t i = start_pixel_x; i < end_pixel_x ; i++)
  {
    for (uint8_t j = start_pixel_y; j < end_pixel_y; j++)
    {
      GLIB_drawPixel(&glib_context, i, j);
    }
  }

  // Draw progress bar
  start_pixel_x = PROGRESS_BAR_EDGE_SIZE + PROGRESS_BAR_SPACE_OFFSET;
  end_pixel_x = start_pixel_x + progress_bar_x;
  start_pixel_y = max_y_size - PROGRESS_BAR_HEIGHT;
  end_pixel_y = max_y_size;

  for (uint8_t i = start_pixel_x; i < end_pixel_x; i++)
  {
    for (uint8_t j = start_pixel_y; j < end_pixel_y; j++)
    {
      GLIB_drawPixel(&glib_context, i, j);
    }
  }

  // Clear the remaining area
  start_pixel_x = PROGRESS_BAR_EDGE_SIZE + PROGRESS_BAR_SPACE_OFFSET
                  + progress_bar_x;
  end_pixel_x = max_x_size - PROGRESS_BAR_EDGE_SIZE - PROGRESS_BAR_SPACE_OFFSET;
  start_pixel_y = max_y_size - PROGRESS_BAR_HEIGHT;
  end_pixel_y = max_y_size;

  for (uint8_t i = start_pixel_x; i < end_pixel_x; i++)
  {
    for (uint8_t j = start_pixel_y; j < end_pixel_y; j++)
    {
      GLIB_drawPixelColor(&glib_context, i, j, White);
    }
  }

  cs_initiator_display_update();
}

/*******************************************************************************
 * Update the display.
 ******************************************************************************/
void cs_initiator_display_update(void)
{
  DMD_updateDisplay();
}

static void cs_initiator_display_measurement_unit(uint8_t x_offset,
                                                  uint8_t y_offset)
{
// Meters
#if (MEASUREMENT_UNIT_METERS == CS_MEASUREMENT_UNIT)

  GLIB_setFont(&glib_context, (GLIB_Font_t *) &GLIB_Font16x16);
  GLIB_drawStringOnLine(&glib_context,
                        "0",   // "0" means the first character of the fonts = 'm'
                        0,
                        GLIB_ALIGN_LEFT,
                        x_offset,
                        y_offset,
                        false);
// Feet
#elif (MEASUREMENT_UNIT_FEET == CS_MEASUREMENT_UNIT)

  GLIB_setFont(&glib_context, (GLIB_Font_t *) &GLIB_Font8x16);
  GLIB_drawStringOnLine(&glib_context,
                        "01",   // "01" means the first two characters of the fonts = "ft"
                        0,
                        GLIB_ALIGN_LEFT,
                        x_offset,
                        y_offset,
                        false);
#endif

  cs_initiator_display_update();
}

static void cs_initiator_display_dot_character(uint8_t x_offset,
                                               uint8_t y_offset)
{
  for (uint8_t i = x_offset; i < (x_offset + DOT_X_SIZE); i++)
  {
    for (uint8_t j = y_offset; j < (y_offset + DOT_Y_SIZE); j++)
    {
      GLIB_drawPixel(&glib_context, i, j);
    }
  }
}

static void cs_initiator_display_clear_distance_area(void)
{
  uint8_t start_pixel = DISTANCE_DISPLAY_Y_OFFSET;
  uint8_t end_pixel = glib_context.font.fontHeight + DISTANCE_DISPLAY_Y_OFFSET;

  for (uint8_t i = 0; i < (glib_context.pDisplayGeometry->xSize); i++)
  {
    for (uint8_t j = start_pixel; j < end_pixel; j++)
    {
      GLIB_drawPixelColor(&glib_context, i, j, White);
    }
  }
}
