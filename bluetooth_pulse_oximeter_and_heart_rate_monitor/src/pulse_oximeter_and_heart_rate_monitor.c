/***************************************************************************//**
 * @file pulse_oximeter_and_heart_rate_monitor
 * @brief pulse_oximeter_and_heart_rate_monitor application source
 * @version 1.0
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
#include "pulse_oximeter_and_heart_rate_monitor.h"
#include "sl_bluetooth.h"
#include "app_assert.h"
#include "app_log.h"
#include "sl_sleeptimer.h"
#include "sparkfun_max30101_max32664.h"
#include "sl_i2cspm_instances.h"
#include "string.h"

#define APP_USER_CONFIG_NVM_ADD          (SL_BT_NVM_KEY_RANGE_USER_MIN)
#define READING_INTERVAL_MS              (500)

#define SPO2_ALARM_CONFIRM_DELAY_MS      (3000)
#define LOW_HR_ALARM_CONFIRM_DELAY_MS    (3000)
#define HIGH_HR_ALARM_CONFIRM_DELAY_MS   (3000)

static user_config_t  gconfig;
static pom_and_hr_monitor_data_t gpom_and_hr_monitor;

static bio_hub_data_t bio_hub_data;
static sl_sleeptimer_timer_handle_t app_timer_handle;
static sl_sleeptimer_timer_handle_t spo2_alarm_confirm_timer_handle;
static sl_sleeptimer_timer_handle_t low_hr_alarm_confirm_timer_handle;
static sl_sleeptimer_timer_handle_t high_hr_alarm_confirm_timer_handle;

static void sensor_monitor_callback(sl_sleeptimer_timer_handle_t *handle,
                                    void *data);
static void spo2_alarm_confirm_timer_callback(
  sl_sleeptimer_timer_handle_t *handle,
  void *data);
static void low_hr_alarm_confirm_timer_callback(
  sl_sleeptimer_timer_handle_t *handle,
  void *data);
static void high_hr_alarm_confirm_timer_callback(
  sl_sleeptimer_timer_handle_t *handle,
  void *data);

static void sensor_monitor_data_reading();
static void sensor_monitor_data_process(void);

sl_status_t pulse_oximeter_and_heart_rate_monitor_init(
  const user_config_t **config,
  const pom_and_hr_monitor_data_t **data)
{
  if ((config == NULL) || (data == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }
  size_t out_len;

  app_log("Pulse Oximeter And Heart Rate Monitor !!!\r\n");
  sl_status_t ret_code = sl_bt_nvm_load(APP_USER_CONFIG_NVM_ADD,
                                        sizeof(gconfig),
                                        &out_len,
                                        (uint8_t *)&gconfig);

  if (ret_code == SL_STATUS_NOT_FOUND) {
    app_log("No config data for pulse oximeter and heart rate monitor !!!\r\n");
    app_log(
      "Please connect with the  EFR Mobile app, then set the config data\r\n");
  } else if (ret_code == SL_STATUS_OK) {
    app_log("-------------------------------------------\r\n");
    app_log("Load config from NVM\r\n");
    app_log("alarm_enabled: %d\r\n", gconfig.alarm_enabled);
    app_log("buzzer_volume: %d\r\n", gconfig.buzzer_volume);
    app_log("threshold_spo2: %d\r\n", gconfig.threshold_spo2);
    app_log("threshold_hr_low: %d\r\n", gconfig.threshold_hr_low);
    app_log("threshold_hr_high: %d\r\n", gconfig.threshold_hr_high);
    app_log("-------------------------------------------\r\n");
  } else {
    app_assert_status(SL_STATUS_FAIL);
  }

  /* ---------- Init max30101 and max32664 ---------- */
  uint16_t pulse_width;
  uint16_t sample_val;

  ret_code = bio_hub_init(sl_i2cspm_qwiic, 0);
  app_assert_status(ret_code);

  // Configuring just the BPM settings.
  ret_code = bio_hub_config_bpm(BIO_HUB_ALGO_MODE_TWO);
  app_assert_status(ret_code);

  // Set pulse width.
  ret_code = bio_hub_set_pulse_width(411);
  app_assert_status(ret_code);

  // Check that the pulse width was set.
  ret_code = bio_hub_read_pulse_width(&pulse_width);
  app_assert_status(ret_code);

  // Set sample rate per second. Remember that not every sample rate is
  // available with every pulse width. Check hookup guide for more information.
  ret_code = bio_hub_set_sample_rate(400);
  app_assert_status(ret_code);

  // Check sample rate.
  ret_code = bio_hub_read_sample_rate(&sample_val);
  app_assert_status(ret_code);

  *config = &gconfig;
  *data = &gpom_and_hr_monitor;

  app_log("The Biosensor (max30101 and max32664) init successfully\r\n");

  // starts a periodic timer to get data from sensor
  sl_sleeptimer_start_periodic_timer_ms(&app_timer_handle,
                                        READING_INTERVAL_MS,
                                        sensor_monitor_callback,
                                        (void *) NULL,
                                        0,
                                        0);
  return SL_STATUS_OK;
}

void pulse_oximeter_and_heart_rate_monitor_timer_event_handler(void)
{
  sensor_monitor_data_reading();
  sensor_monitor_data_process();
}

sl_status_t pulse_oximeter_and_heart_rate_monitor_set_user_config(
  set_user_config_flag_t config_flag,
  uint8_t value)
{
  sl_status_t ret_code = SL_STATUS_OK;

  // Validate the config value
  switch (config_flag) {
    case SET_USER_CONFIG_FLAG_ALARM_ENABLE:
      if ((value != 0) && (value != 1)) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      gconfig.alarm_enabled = value;
      break;
    case SET_USER_CONFIG_FLAG_BUZZER_VOLUME:
      if (value > 10) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      gconfig.buzzer_volume = value;
      break;
    case SET_USER_CONFIG_FLAG_THRESHOLD_SPO2:
      gconfig.threshold_spo2 = value;
      break;
    case SET_USER_CONFIG_FLAG_THRESHOLD_HR_LOW:
      gconfig.threshold_hr_low = value;
      break;
    case SET_USER_CONFIG_FLAG_THRESHOLD_HR_HIGH:
      gconfig.threshold_hr_high = value;
      break;
    default:
      ret_code = SL_STATUS_INVALID_CONFIGURATION;
      break;
  }

  if (ret_code != SL_STATUS_OK) {
    return ret_code;
  }

  // Save the config to the nvm
  ret_code = sl_bt_nvm_save(APP_USER_CONFIG_NVM_ADD,
                            sizeof(user_config_t),
                            (const uint8_t *)&gconfig);
  app_assert_status(ret_code);

  return SL_STATUS_OK;
}

static void sensor_monitor_callback(sl_sleeptimer_timer_handle_t *handle,
                                    void *data)
{
  (void) handle;
  (void) data;

  sl_bt_external_signal(MEASUREMENT_EVENT);
}

static void sensor_monitor_data_reading(void)
{
  /* Read the bio sensor data */
  if (SL_STATUS_OK == bio_hub_read_bpm(&bio_hub_data)) {
    app_log("Heartrate: %d\r\n", bio_hub_data.heart_rate);
    app_log("Confidence: %d\r\n", bio_hub_data.confidence);
    app_log("Oxygen: %d\r\n", bio_hub_data.oxygen);
    app_log("Finger Status: %d\r\n", bio_hub_data.status);
    app_log("Ext Status: %d\r\n", bio_hub_data.ext_status);
    app_log("----------------------\r\n");

    gpom_and_hr_monitor.status.body = bio_hub_data.status;
    gpom_and_hr_monitor.status.body_ext = bio_hub_data.ext_status;

    if ((gpom_and_hr_monitor.status.body == FINGER_DETECTED)
        && (gpom_and_hr_monitor.status.body_ext == EXT_SUCCESS)) {
      gpom_and_hr_monitor.biodata.heart_rate = bio_hub_data.heart_rate;
      gpom_and_hr_monitor.biodata.spo2 = bio_hub_data.oxygen;
      gpom_and_hr_monitor.biodata.confidence = bio_hub_data.confidence;
    } else {
      gpom_and_hr_monitor.biodata.heart_rate = 0;
      gpom_and_hr_monitor.biodata.spo2 = 0;
      gpom_and_hr_monitor.biodata.confidence = 0;
    }
  }
}

static void sensor_monitor_data_process(void)
{
  sl_status_t sc;

  if ((gconfig.alarm_enabled == true)
      && (gpom_and_hr_monitor.status.body == FINGER_DETECTED)
      && (gpom_and_hr_monitor.status.body_ext == EXT_SUCCESS)) {
    // Checks the Spo2 values against the configured thresholds
    if (gpom_and_hr_monitor.biodata.spo2 <= gconfig.threshold_spo2) {
      if (gpom_and_hr_monitor.status.spo2_alarm == false) {
        sl_sleeptimer_start_timer_ms(&spo2_alarm_confirm_timer_handle,
                                     SPO2_ALARM_CONFIRM_DELAY_MS,
                                     spo2_alarm_confirm_timer_callback,
                                     (void *) NULL,
                                     0,
                                     0);
      }
    } else {
      gpom_and_hr_monitor.status.spo2_alarm = false;
      bool spo2_timer_running;
      sc = sl_sleeptimer_is_timer_running(&spo2_alarm_confirm_timer_handle,
                                          &spo2_timer_running);
      app_assert_status(sc);

      if (spo2_timer_running) {
        sl_sleeptimer_stop_timer(&spo2_alarm_confirm_timer_handle);
      }
    }

    // Checks the heart rate values against the configured thresholds
    if (gpom_and_hr_monitor.biodata.heart_rate <= gconfig.threshold_hr_low) {
      if (gpom_and_hr_monitor.status.low_heart_rate_alarm == false) {
        sl_sleeptimer_start_timer_ms(&low_hr_alarm_confirm_timer_handle,
                                     LOW_HR_ALARM_CONFIRM_DELAY_MS,
                                     low_hr_alarm_confirm_timer_callback,
                                     (void *) NULL,
                                     0,
                                     0);
      }
    } else {
      gpom_and_hr_monitor.status.low_heart_rate_alarm = false;
      bool low_hr_timer_running;
      sc = sl_sleeptimer_is_timer_running(&low_hr_alarm_confirm_timer_handle,
                                          &low_hr_timer_running);
      app_assert_status(sc);

      if (low_hr_timer_running) {
        sl_sleeptimer_stop_timer(&low_hr_alarm_confirm_timer_handle);
      }
    }

    if (gpom_and_hr_monitor.biodata.heart_rate >= gconfig.threshold_hr_high) {
      if (gpom_and_hr_monitor.status.high_heart_rate_alarm == false) {
        sl_sleeptimer_start_timer_ms(&high_hr_alarm_confirm_timer_handle,
                                     HIGH_HR_ALARM_CONFIRM_DELAY_MS,
                                     high_hr_alarm_confirm_timer_callback,
                                     (void *) NULL,
                                     0,
                                     0);
      }
    } else {
      gpom_and_hr_monitor.status.high_heart_rate_alarm = false;
      bool high_hr_timer_running;
      sc = sl_sleeptimer_is_timer_running(&high_hr_alarm_confirm_timer_handle,
                                          &high_hr_timer_running);
      app_assert_status(sc);

      if (high_hr_timer_running) {
        sl_sleeptimer_stop_timer(&high_hr_alarm_confirm_timer_handle);
      }
    }
  } else {
    gpom_and_hr_monitor.status.spo2_alarm = false;
    gpom_and_hr_monitor.status.low_heart_rate_alarm = false;
    gpom_and_hr_monitor.status.high_heart_rate_alarm = false;
  }
}

static void spo2_alarm_confirm_timer_callback(
  sl_sleeptimer_timer_handle_t *handle,
  void *data)
{
  (void) handle;
  (void) data;
  gpom_and_hr_monitor.status.spo2_alarm = true;
}

static void low_hr_alarm_confirm_timer_callback(
  sl_sleeptimer_timer_handle_t *handle,
  void *data)
{
  (void) handle;
  (void) data;
  gpom_and_hr_monitor.status.low_heart_rate_alarm = true;
}

static void high_hr_alarm_confirm_timer_callback(
  sl_sleeptimer_timer_handle_t *handle,
  void *data)
{
  (void) handle;
  (void) data;
  gpom_and_hr_monitor.status.high_heart_rate_alarm = true;
}
