/***************************************************************************//**
 * @file cs_presence_detection.c
 * @brief CS Based Presence Detection
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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "sl_component_catalog.h"
#include "sl_simple_led_instances.h"
#include "sl_bt_api.h"
#include "app_log.h"
#include "gatt_db.h"
#include "cs_presence_detection.h"

#ifdef SL_CATALOG_DMD_MEMLCD_PRESENT
#include "glib.h"
#include "dmd.h"

#define DISTANCE_THRESHOLD_LINE           (1)
#define PRESENCE_DETECT_STATUS_LINE       (2)

static GLIB_Context_t glib_context;
static const char *cs_presence_detection_msg[] =
{ (const char *)"Presence", (const char *)"No Presence" };

/*******************************************************************************
 * Update the display.
 ******************************************************************************/
static sl_status_t cs_presence_detection_display_update(void);
static sl_status_t cs_presence_detection_display_threshold(void);
static sl_status_t cs_presence_detection_display_clear_row(uint8_t row);
static sl_status_t cs_presence_detection_display_information(
  cs_presence_detection_t presence_detect);

#endif

#if (MEASUREMENT_UNIT_METERS == CS_MEASUREMENT_UNIT) // Meters
#elif (MEASUREMENT_UNIT_FEET == CS_MEASUREMENT_UNIT) // Feet
#define METER_TO_FEET_CONSTANT_VALUE  (3.28084F)
#endif

static cs_presence_detection_t presence_detect = CS_PRESENCE_DETECTED;

/*******************************************************************************
 * cs_presence_detection_init.
 ******************************************************************************/
sl_status_t cs_presence_detection_init(void)
{
#ifdef SL_CATALOG_DMD_MEMLCD_PRESENT
  EMSTATUS status = DMD_init(0);
  if (status != DMD_OK) {
    return SL_STATUS_INITIALIZATION;
  }

  status = GLIB_contextInit(&glib_context);
  if (status != GLIB_OK) {
    return SL_STATUS_INITIALIZATION;
  }

  glib_context.backgroundColor = White;
  glib_context.foregroundColor = Black;
  GLIB_clear(&glib_context);

  cs_presence_detection_display_threshold();
  cs_presence_detection_display_information(presence_detect);

  // force update LCD content for drawing the first screen
  cs_presence_detection_display_update();
#endif

  // Meters
#if (MEASUREMENT_UNIT_METERS == CS_MEASUREMENT_UNIT)
  printf("Distance Threshold: %02um\r\n", DISTANCE_THRESHOLD);
  // Feet
#elif (MEASUREMENT_UNIT_FEET == CS_MEASUREMENT_UNIT)
  app_log("Distance Threshold: %02uft\r\n", DISTANCE_THRESHOLD);
#endif

  presence_detect = CS_PRESENCE_DETECTED;

  sl_led_turn_on(&sl_led_led0);

  return SL_STATUS_OK;
}

/*******************************************************************************
 * cs_presence_detection_processing
 * Processing distance value to detect presence.
 ******************************************************************************/
sl_status_t cs_presence_detection_processing(float measured_distance)
{
#if (MEASUREMENT_UNIT_METERS == CS_MEASUREMENT_UNIT) // Meters
#elif (MEASUREMENT_UNIT_FEET == CS_MEASUREMENT_UNIT) // Feet
  measured_distance = measured_distance * METER_TO_FEET_CONSTANT_VALUE;
#endif
  sl_status_t stt = SL_STATUS_OK;

  if ((uint16_t)measured_distance >= DISTANCE_THRESHOLD) {
    if (presence_detect == CS_PRESENCE_DETECTED) {
      presence_detect = CS_PRESENCE_NOT_DETECTED;
      sl_led_turn_off(&sl_led_led0);
      app_log("PRESENCE NOT DETECTED\r\n");
    }
  } else { // measured_distance < DISTANCE_THRESHOLD
    if (presence_detect == CS_PRESENCE_NOT_DETECTED) {
      presence_detect = CS_PRESENCE_DETECTED;
      sl_led_turn_on(&sl_led_led0);
      app_log("PRESENCE DETECTED\r\n");
    }
  }

#ifdef SL_CATALOG_DMD_MEMLCD_PRESENT
  cs_presence_detection_display_information(presence_detect);
#endif

  return stt;
}

#ifdef SL_CATALOG_DMD_MEMLCD_PRESENT

/*******************************************************************************
 * cs_presence_detection_display_information
 ******************************************************************************/
sl_status_t cs_presence_detection_display_information(
  cs_presence_detection_t presence_detect)
{
  if (presence_detect < CS_PRESENCE_MAX) {
    cs_presence_detection_display_clear_row(PRESENCE_DETECT_STATUS_LINE);
    GLIB_drawStringOnLine(&glib_context,
                          (const char *)cs_presence_detection_msg[
                            presence_detect],
                          PRESENCE_DETECT_STATUS_LINE,
                          GLIB_ALIGN_LEFT,
                          0,
                          0,
                          false);

    cs_presence_detection_display_update();
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

/*******************************************************************************
 * cs_presence_detection_display_update.
 ******************************************************************************/
static sl_status_t cs_presence_detection_display_update(void)
{
  DMD_updateDisplay();
  return SL_STATUS_OK;
}

static sl_status_t cs_presence_detection_display_threshold(void)
{
  char buffer[20];
// Meters
#if (MEASUREMENT_UNIT_METERS == CS_MEASUREMENT_UNIT)
  sprintf(buffer, "Threshold: %02um", DISTANCE_THRESHOLD);
// Feet
#elif (MEASUREMENT_UNIT_FEET == CS_MEASUREMENT_UNIT)
  sprintf(buffer, "Threshold: %02uft", DISTANCE_THRESHOLD);
#endif

  GLIB_drawStringOnLine(&glib_context,
                        (const char *)buffer,
                        DISTANCE_THRESHOLD_LINE,
                        GLIB_ALIGN_LEFT,
                        0,
                        0,
                        false);

  return SL_STATUS_OK;
}

static sl_status_t cs_presence_detection_display_clear_row(uint8_t row)
{
  const char *empty = "                ";
  GLIB_drawStringOnLine(&glib_context,
                        empty,
                        row,
                        GLIB_ALIGN_LEFT,
                        0,
                        0,
                        true);
  return SL_STATUS_OK;
}

#endif //SL_CATALOG_DMD_MEMLCD_PRESENT
