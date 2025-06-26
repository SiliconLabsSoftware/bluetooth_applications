/***************************************************************************//**
 * @file app_ph2.c
 * @brief Main application logic.
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
#include "app_ph2.h"
#include "mikroe_ph2.h"
#include "sl_bluetooth.h"
#include "sl_sleeptimer.h"
#include "app_log.h"
#include "sl_i2cspm_instances.h"

static volatile bool trigger_process = false;
uint8_t calibrated = false;
static mikroe_i2c_handle_t app_i2c_instance = NULL;
static sl_sleeptimer_timer_handle_t app_timer_handle;
static void app_timer_callback(sl_sleeptimer_timer_handle_t *handle,
                               void *data);

void app_mikroe_ph2_init(void)
{
  app_i2c_instance = sl_i2cspm_qwiic;

  sl_status_t stt;

  stt = sl_bt_nvm_load(CALIBRATED_NVM_ADDRESS,
                       sizeof(calibrated),
                       NULL,
                       &calibrated);

  stt = mikroe_ph2_init(app_i2c_instance);
  app_assert_status(stt);

  app_log("======== Pond water pH measurement example ========\r\n");

  if (!calibrated) {
    app_mikroe_ph2_calibrate();
  } else {
    app_log("Offset calibration has already been completed!\r\n");
  }

  app_log("Please reconnect the probe within 5 seconds and immerse it in a"
          " pH-neutral solution (such as distilled water) to perform midpoint"
          " calibration.\r\nFailure to do so may result in inaccurate readings"
          " due to the use of an inappropriate substance for"
          " the neutral reference point.\r\n");
  sl_sleeptimer_delay_millisecond(5000);
  app_log("Setting neutral pH value...\r\n");

  stt = mikroe_ph2_calibrate(NEUTRAL_PH_VALUE);
  app_assert_status(stt);
  app_log("pH Calibration is completed! \r\n");
  app_log("Set up periodic sleeptimer with callback 0x%lx\r\n",
          sl_sleeptimer_start_periodic_timer_ms(&app_timer_handle,
                                                READING_INTERVAL_MSEC,
                                                app_timer_callback,
                                                NULL,
                                                0,
                                                0));

  app_log("Successfully initialized!\r\n");
}

void app_mikroe_ph2_calibrate(void)
{
  sl_status_t stt;
  app_log("Please perform calibration by:\r\n");
  app_log("   - Step 1: Disconnect the BNC connector,"
          " and then 'short-circuit' it\r\n");
  app_log("   - Step 2: Adjust the offset potentiometer"
          " depending on the STAT LEDs\r\n");
  app_log("     * STAT1 Blink --> turn it clockwise\r\n");
  app_log("     * STAT2 Blink --> turn it counter-clockwise\r\n");
  app_log("     * Keep turning it until the LEDs stop blinking,"
          " make sure during the whole process, it is 'short-circuited'\n");
  stt = mikroe_ph2_calibrate_offset();
  app_assert_status(stt);

  app_log("Offset calibration is completed!\r\n");

  app_log("Please reconnect the probe within 5 seconds and immerse it in a"
          " pH-neutral solution (such as distilled water) to perform midpoint"
          " calibration.\r\nFailure to do so may result in inaccurate readings"
          " due to the use of an inappropriate substance for"
          " the neutral reference point.\r\n");
  sl_sleeptimer_delay_millisecond(5000);
  app_log("Setting neutral pH value...\r\n");

  stt = mikroe_ph2_calibrate(NEUTRAL_PH_VALUE);
  app_assert_status(stt);
  app_log("pH Calibration is completed! \r\n");

  calibrated = true;
  sl_bt_nvm_save(CALIBRATED_NVM_ADDRESS,
                 sizeof(calibrated),
                 &calibrated);
}

void app_mikroe_ph2_process(float *pH_value)
{
  if (trigger_process) {
    trigger_process = false;

    sl_status_t stt = mikroe_ph2_calculate_ph(pH_value);
    if (SL_STATUS_OK == stt) {
      app_log("pH value: %.2f \r\n", *pH_value);
    }
  }
}

static void app_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void) data;
  (void) handle;

  trigger_process = true;
}
