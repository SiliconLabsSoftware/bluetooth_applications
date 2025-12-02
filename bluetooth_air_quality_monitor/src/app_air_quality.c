/***************************************************************************//**
 * @file app_air_quality.c
 * @brief Air Quality demo using CCS811 sensor
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
#include <stdio.h>
#include <string.h>

#include "sl_simple_button_instances.h"
#include "sl_i2cspm_instances.h"
#include "sl_pwm_instances.h"
#include "gatt_db.h"
#include "app_assert.h"
#include "app_log.h"
#include "app_timer.h"

#include "micro_oled_ssd1306.h"
#include "mikroe_cmt_8540s_smt.h"
#include "buzz2.h"

#include "glib.h"
#include "app_nvm3.h"
#include "app_sensor.h"
#include "app_buzzer.h"
#include "app_display.h"
#include "app_air_quality.h"

uint8_t air_quality_connection = 0xff;
// Application data
static app_air_quality_t air_quality;

// Periodic timer handle.
static app_timer_t  air_quality_timer;
// Periodic timer callback.
static void air_quality_timer_cb(app_timer_t *handle, void *data);
static air_quality_index_t air_quality_co2_index(uint16_t data);
static air_quality_index_t air_quality_tvoc_index(uint16_t data);
static void air_quality_data_process(sensor_data_t data);
static void air_quality_alarm_process(void);
static uint16_t air_quality_moving_average(uint16_t *data);
static void co2_notify(void);
static void tvoc_notify(void);
static void aqi_notify(void);
static void alarm_status_notify(void);

/***************************************************************************//**
 * Initialize the AIR QUALITY application.
 ******************************************************************************/
void air_quality_init(void)
{
  sl_status_t stt;

  air_quality.data.is_buffer_full = false;
  air_quality.data.samples_counter = 0;

  // Initialize and Load configuration from NVM
  app_log_info("Loading parameters from NVM...\r\n");
  stt = nvm3_user_init(&air_quality.config);
  if (stt != SL_STATUS_OK) {
    app_log_warning("Load configurations from NVM3 failed\r\n");
  }
  app_log_info("Loaded from NVM. Alarm Enabled = %d\r\n",
               air_quality.config.alarm_enabled);
  app_log_info("Loaded from NVM. CO2 Threshold = %d\r\n",
               air_quality.config.co2_threshold);
  app_log_info("Loaded from NVM. tVOC Threshold = %d\r\n",
               air_quality.config.tvoc_threshold);
  app_log_info("Loaded from NVM. Measurement Interval = %d\r\n",
               air_quality.config.measurement_interval);
  app_log_info("Loaded from NVM. Buzzer volume = %d\r\n",
               air_quality.config.buzzer_volume);

  stt = oled_init();
  if (stt != SL_STATUS_OK) {
    app_log_warning("OLED display initialization failed\r\n");
  }
  app_log_info("OLED display initialization successfully\r\n");

  stt = buzzer_init();
  if (stt != SL_STATUS_OK) {
    app_log_warning("Buzzer initialization failed\r\n");
  }
  app_log_info("Buzzer initialization successfully\r\n");
  buzzer_set_volume(air_quality.config.buzzer_volume);
  app_log_info("Buzzer volume is set to = %d\r\n",
               air_quality.config.buzzer_volume);

  stt = sensor_init();
  if (stt != SL_STATUS_OK) {
    app_log_warning("Environment sensor initialization failed\r\n");
  }
  app_log_info("Environment sensor initialization successfully\r\n");

  // Start timer used for periodic measurement.
  app_log_info("Starting timer used for periodic measurement.\r\n");
  app_timer_start(&air_quality_timer,
                  air_quality.config.measurement_interval * 1000,
                  air_quality_timer_cb,
                  NULL,
                  true);
}

/***************************************************************************//**
 * Process Bluetooth external events.
 ******************************************************************************/
void air_quality_external_signal_cb(uint32_t event_flags)
{
  if (event_flags & AIR_QUALITY_MONITOR_BUTTON_EVENT) {
    if (air_quality.config.alarm_enabled) {
      // disable alarm
      air_quality.config.alarm_enabled = 0;     // 0 - disable
      nvm3_user_set_alarm_enabled(air_quality.config.alarm_enabled);
      // deactivate buzzer if it is activating
      if (air_quality.data.alarm_status) {
        buzzer_control(0);
      }
    } else {
      // enable alarm
      air_quality.config.alarm_enabled = 1;     // 1 - enable
      nvm3_user_set_alarm_enabled(air_quality.config.alarm_enabled);
      // activate buzzer if alarm is activating
      if (air_quality.data.alarm_status) {
        buzzer_control(1);
      }
    }
    app_log_info("Alarm Enabled: %d\r\n", air_quality.config.alarm_enabled);
  }
}

/***************************************************************************//**
 * Simple Button
 * Button state changed callback
 * @param[in] handle
 *    Button event handle
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  // Button released.
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
    if (&sl_button_btn0 == handle) {
      sl_bt_external_signal(AIR_QUALITY_MONITOR_BUTTON_EVENT);
    }
  }
}

/*******************************************************************************
 *   Function to handle read data
 ******************************************************************************/
void air_quality_user_read_cb(
  sl_bt_evt_gatt_server_user_read_request_t *data)
{
  sl_status_t stt;
  uint16_t characteristic_size = 0;
  const uint8_t *characteristic_ptr = NULL;
  uint16_t sent_len;

  switch (data->characteristic) {
    // Notification enabled characteristic
    case gattdb_alarm_enabled:
      characteristic_size = sizeof(air_quality.config.alarm_enabled);
      characteristic_ptr =
        (const uint8_t *) &air_quality.config.alarm_enabled;
      break;

    // Buzzer volume characteristic
    case gattdb_buzzer_volume:
      characteristic_size = sizeof(air_quality.config.buzzer_volume);
      characteristic_ptr =
        (const uint8_t *) &air_quality.config.buzzer_volume;
      break;

    // Measurement period characteristic
    case gattdb_measurement_interval:
      characteristic_size = sizeof(air_quality.config.measurement_interval);
      characteristic_ptr =
        (const uint8_t *) &air_quality.config.measurement_interval;
      break;

    // CO2 threshold characteristic
    case gattdb_co2_threshold:
      characteristic_size = sizeof(air_quality.config.co2_threshold);
      characteristic_ptr =
        (const uint8_t *) &air_quality.config.co2_threshold;
      break;

    // tVOC threshold characteristic
    case gattdb_tvoc_threshold:
      characteristic_size = sizeof(air_quality.config.tvoc_threshold);
      characteristic_ptr =
        (const uint8_t *) &air_quality.config.tvoc_threshold;
      break;

    // CO2 characteristic
    case gattdb_co2:
      characteristic_size = sizeof(air_quality.data.co2);
      characteristic_ptr = (const uint8_t *) &air_quality.data.co2;
      break;

    // tVOC characteristic
    case gattdb_tvoc:
      characteristic_size = sizeof(air_quality.data.tvoc);
      characteristic_ptr = (const uint8_t *) &air_quality.data.tvoc;
      break;

    // Notification Status characteristic
    case gattdb_alarm_status:
      characteristic_size = sizeof(air_quality.data.alarm_status);
      characteristic_ptr = (const uint8_t *) &air_quality.data.alarm_status;
      break;

    // Air Quality Index characteristic
    case gattdb_aqi:
      characteristic_size = sizeof(air_quality.data.aqi);
      characteristic_ptr = (const uint8_t *) &air_quality.data.aqi;
      break;

    // Do nothing
    default:
      break;
  }

  // Send response
  stt = sl_bt_gatt_server_send_user_read_response(data->connection,
                                                  data->characteristic,
                                                  (uint8_t) 0x00, // SUCCESS
                                                  characteristic_size,
                                                  characteristic_ptr,
                                                  &sent_len);
  app_assert_status(stt);
}

/*******************************************************************************
 *   Function to handle write data
 ******************************************************************************/
void air_quality_user_write_cb(
  sl_bt_evt_gatt_server_user_write_request_t *data)
{
  sl_status_t response_code = 0;
  sl_status_t stt;

  switch (data->characteristic) {
    // Notification characteristic written
    case gattdb_alarm_enabled:
      if (data->value.len == 1) {
        uint8_t value = *(data->value.data);
        stt = nvm3_user_set_alarm_enabled(value);
        if (stt == SL_STATUS_OK) {
          response_code = SL_STATUS_OK;
          air_quality.config.alarm_enabled = value;
          app_log_info("User write request -> Alarm Enabled: %d\r\n", value);
        } else if (stt == SL_STATUS_INVALID_RANGE) {
          response_code = SL_STATUS_BT_ATT_OUT_OF_RANGE; // The attribute value is out of range
        } else {
          response_code = SL_STATUS_BT_ATT_WRITE_REQUEST_REJECTED; // The requested write operation cannot be fulfilled
        }
      } else {
        response_code = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH; // Invalid Attribute Value Length
      }
      break;

    case gattdb_buzzer_volume:
      if (data->value.len == 1) {
        uint8_t value = *(data->value.data);
        stt = nvm3_user_set_buzzer_volume(value);
        if (stt == SL_STATUS_OK) {
          response_code = SL_STATUS_OK;
          air_quality.config.buzzer_volume = value;
          app_log_info("User write request -> Buzzer Volume: %d\r\n", value);
          buzzer_set_volume(air_quality.config.buzzer_volume);
          app_log_info("Buzzer volume is set to = %d\r\n",
                       air_quality.config.buzzer_volume);
        } else if (stt == SL_STATUS_INVALID_RANGE) {
          response_code = SL_STATUS_BT_ATT_OUT_OF_RANGE; // The attribute value is out of range
        } else {
          response_code = SL_STATUS_BT_ATT_WRITE_REQUEST_REJECTED; // The requested write operation cannot be fulfilled
        }
      } else {
        response_code = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH; // Invalid Attribute Value Length
      }
      break;

    case gattdb_co2_threshold:
      if (data->value.len == 2) {
        uint16_t value = *((uint16_t *)data->value.data);
        stt = nvm3_user_set_co2_threshold(value);
        if (stt == SL_STATUS_OK) {
          response_code = SL_STATUS_OK;
          air_quality.config.co2_threshold = value;
          app_log_info("User write request -> CO2 Threshold: %d\r\n",
                       value);
        } else if (stt == SL_STATUS_INVALID_RANGE) {
          response_code = SL_STATUS_BT_ATT_OUT_OF_RANGE; // The attribute value is out of range
        } else {
          response_code = SL_STATUS_BT_ATT_WRITE_REQUEST_REJECTED; // The requested write operation cannot be fulfilled
        }
      } else {
        response_code = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH; // Invalid Attribute Value Length
      }
      break;

    case gattdb_tvoc_threshold:
      if (data->value.len == 2) {
        uint16_t value = *((uint16_t *)data->value.data);
        stt = nvm3_user_set_tvoc_threshold(value);
        if (stt == SL_STATUS_OK) {
          response_code = SL_STATUS_OK;
          air_quality.config.tvoc_threshold = value;
          app_log_info("User write request -> tVOC Threshold: %d\r\n",
                       value);
        } else if (stt == SL_STATUS_INVALID_RANGE) {
          response_code = SL_STATUS_BT_ATT_OUT_OF_RANGE; // The attribute value is out of range
        } else {
          response_code = SL_STATUS_BT_ATT_WRITE_REQUEST_REJECTED; // The requested write operation cannot be fulfilled
        }
      } else {
        response_code = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH; // Invalid Attribute Value Length
      }
      break;

    case gattdb_measurement_interval:
      if (data->value.len == 1) {
        uint8_t value = *(data->value.data);
        stt = nvm3_user_set_measurement_interval(value);
        if (stt == SL_STATUS_OK) {
          response_code = SL_STATUS_OK;
          air_quality.config.measurement_interval = value;
          app_log_info("User write request -> Measurement Interval: %d\r\n",
                       value);
          // Re-Start timer used for periodic operations with new interval
          app_timer_start(&air_quality_timer,
                          air_quality.config.measurement_interval * 1000,
                          air_quality_timer_cb,
                          NULL,
                          true);
        } else if (stt == SL_STATUS_INVALID_RANGE) {
          response_code = SL_STATUS_BT_ATT_OUT_OF_RANGE; // The attribute value is out of range
        } else {
          response_code = SL_STATUS_BT_ATT_WRITE_REQUEST_REJECTED; // The requested write operation cannot be fulfilled
        }
      } else {
        response_code = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH; // Invalid Attribute Value Length
      }
      break;

    // Write operation not permitted by default
    default:
      response_code = (uint8_t) SL_STATUS_BT_ATT_VALUE_NOT_ALLOWED;
      break;
  }

  if (response_code != SL_STATUS_OK) {
    app_log_info("User write request failed, Error Codes: 0x%lx\r\n",
                 response_code);
  }
  // Send write response.
  stt = sl_bt_gatt_server_send_user_write_response(data->connection,
                                                   data->characteristic,
                                                   (uint8_t)response_code);
  app_assert_status(stt);
}

void air_quality_characteristic_status_cb(
  sl_bt_evt_gatt_server_characteristic_status_t *data)
{
  bool enable = sl_bt_gatt_disable != data->client_config_flags;
  void (*notify)(void) = NULL;
  air_quality_connection = data->connection;

  // update notification status
  switch (data->characteristic) {
    case gattdb_co2:
      air_quality.config.notification.co2 = enable;
      notify = &co2_notify;
      break;
    case gattdb_tvoc:
      air_quality.config.notification.tvoc = enable;
      notify = &tvoc_notify;
      break;
    case gattdb_alarm_status:
      air_quality.config.notification.alarm_status = enable;
      notify = &alarm_status_notify;
      break;
    case gattdb_aqi:
      air_quality.config.notification.aqi = enable;
      notify = &aqi_notify;
      break;
    default:
      app_assert(false, "Unexpected characteristic\n");
      break;
  }

  if (enable) {
    // send the first notification
    (*notify)();
  }
}

void air_quality_connection_closed_cb(sl_bt_msg_t *evt)
{
  (void)evt;

  air_quality_connection = 0xff;
  // reset notification flags
  air_quality.config.notification.co2 = false;
  air_quality.config.notification.tvoc = false;
  air_quality.config.notification.aqi = false;
  air_quality.config.notification.alarm_status = false;
}

air_quality_data_t air_quality_get_data(void)
{
  return air_quality.data;
}

/***************************************************************************//**
 * Air quality monitor event handler retrieve and
 * process the measured air quality data.
 ******************************************************************************/
static void air_quality_timer_cb(app_timer_t *handle, void *data)
{
  (void)handle;
  (void)data;
  sensor_data_t measurement_data;

  if (SL_STATUS_OK == sensor_get_data(&measurement_data)) {
    app_log_info(">> Current CO2: %d ppm, Current tVOC: %d ppb\r\n",
                 measurement_data.co2,
                 measurement_data.tvoc);
    air_quality_data_process(measurement_data);
    air_quality_alarm_process();
  }
}

/***************************************************************************//**
 * @brief
 *  This function processes the measured values.
 *  Filters the lowest and greatest values, and calculates an average.
 *
 * @param[in] data
 *  Pointer to data storage structure
 *
 * @return
 *  Returns sample filtered.
 ******************************************************************************/
static uint16_t air_quality_moving_average(uint16_t *data)
{
  uint32_t sum = 0;
  uint16_t min, max;
  uint8_t i, max_count;

  if (!air_quality.data.samples_counter) {
    return 0;
  }

  // Calculate average value
  min = max = data[0];
  max_count = air_quality.data.is_buffer_full
              ? DATA_BUFFER_SIZE : air_quality.data.samples_counter;
  for (i = 0; i < max_count; i++) {
    sum += data[i];
    if (data[i] > max) {
      max = data[i];
    }

    if (data[i] < min) {
      min = data[i];
    }
  }

  // Exclude  min. and max. values
  if (i > 2) {
    i -= 2;
    sum = sum - (min + max);
  }

  return (uint16_t) (sum / i);
}

/***************************************************************************//**
 * This function processes the measured values
 ******************************************************************************/
static void air_quality_data_process(sensor_data_t data)
{
  air_quality_index_t co2_index, tvoc_index;

  uint8_t index = air_quality.data.samples_counter % DATA_BUFFER_SIZE;
  // Store measurement data.
  // appends a new value to the measurement data, removes the oldest one.
  air_quality.data.co2_buffer[index] = data.co2;
  air_quality.data.tvoc_buffer[index] = data.tvoc;
  air_quality.data.samples_counter++;
  if (air_quality.data.samples_counter >= DATA_BUFFER_SIZE) {
    air_quality.data.is_buffer_full = true;
  }

  air_quality.data.co2 =
    air_quality_moving_average(air_quality.data.co2_buffer);
  air_quality.data.tvoc = air_quality_moving_average(
    air_quality.data.tvoc_buffer);

  app_log_info(">> Average CO2: %d ppm, Average tVOC: %d ppb \r\n",
               air_quality.data.co2,
               air_quality.data.tvoc);

  if (air_quality.config.notification.co2) {
    co2_notify();
  }

  if (air_quality.config.notification.tvoc) {
    tvoc_notify();
  }

  // Get status of air quality index.
  co2_index = air_quality_co2_index(air_quality.data.co2);
  tvoc_index = air_quality_tvoc_index(air_quality.data.tvoc);

  // The overall air quality index for indoors is thus
  // based on the worst air quality index rating among them
  air_quality.data.aqi = (co2_index < tvoc_index ? tvoc_index : co2_index);
  if (air_quality.config.notification.aqi) {
    aqi_notify();
  }
}

static void air_quality_alarm_process(void)
{
  // Check thresholds
  if ((air_quality.data.co2 > air_quality.config.co2_threshold)
      || (air_quality.data.tvoc > air_quality.config.tvoc_threshold)) {
    air_quality.data.alarm_status = 1; // 1 - activate
  } else {
    air_quality.data.alarm_status = 0; // 0 - deactivate
  }

  // Turn activate/deactivate buzzer if alarm enabled
  if (air_quality.config.alarm_enabled) {
    buzzer_control(air_quality.data.alarm_status == 1);
  }

  // Send notification if enabled
  if (air_quality.config.notification.alarm_status) {
    alarm_status_notify();
  }
}

/***************************************************************************//**
 * Update the status of air quality index to prepare show on LCD display
 *
 *  Air quality should be indicated on the display (with text, poor, good
 *   etc...) in accordance with these levels below.
 *
 ******************************************************************************/
static air_quality_index_t air_quality_co2_index(uint16_t data)
{
  if (data < 400) {
    return EXCELLENT;
  } else if (data < 1000) {
    return FINE;
  } else if (data < 1500) {
    return MODERATE;
  } else if (data < 2000) {
    return POOR;
  } else if (data < 5000) {
    return VERY_POOR;
  }

  return SEVERE;
}

/***************************************************************************//**
 * Update the status of air quality index to prepare show on LCD display
 *
 *  Air quality should be indicated on the display (with text, poor, good
 *   etc...) in accordance with these levels below.
 *
 ******************************************************************************/
static air_quality_index_t air_quality_tvoc_index(uint16_t data)
{
  if (data < 50) {
    return EXCELLENT;
  } else if (data < 100) {
    return FINE;
  } else if (data < 150) {
    return MODERATE;
  } else if (data < 200) {
    return POOR;
  } else if (data < 300) {
    return VERY_POOR;
  }

  return SEVERE;
}

static void co2_notify(void)
{
  sl_status_t sc;
  sc = sl_bt_gatt_server_send_notification(air_quality_connection,
                                           gattdb_co2,
                                           sizeof(air_quality.data.co2),
                                           (uint8_t *)&air_quality.data.co2);
  app_assert_status(sc);
}

static void tvoc_notify(void)
{
  sl_status_t sc;
  sc = sl_bt_gatt_server_send_notification(air_quality_connection,
                                           gattdb_tvoc,
                                           sizeof(air_quality.data.tvoc),
                                           (uint8_t *)&air_quality.data.tvoc);
  app_assert_status(sc);
}

static void aqi_notify(void)
{
  sl_status_t sc;
  sc = sl_bt_gatt_server_send_notification(air_quality_connection,
                                           gattdb_aqi,
                                           sizeof(air_quality.data.aqi),
                                           (uint8_t *)&air_quality.data.aqi);
  app_assert_status(sc);
}

static void alarm_status_notify(void)
{
  sl_status_t sc;
  sc = sl_bt_gatt_server_send_notification(air_quality_connection,
                                           gattdb_alarm_status,
                                           sizeof(air_quality.data.alarm_status),
                                           (uint8_t *)&air_quality.data.alarm_status);
  app_assert_status(sc);
}
