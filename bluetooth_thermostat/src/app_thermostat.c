/***************************************************************************//**
 * @file app_thermostat.c
 * @brief Thermostat application code
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
#include "sl_bluetooth.h"
#include "sl_udelay.h"
#include "gatt_db.h"
#include "app_log.h"
#include "app_assert.h"
#include "app_timer.h"
#include "sl_simple_button_instances.h"

#include "app_thermostat.h"
#include "app_buzzer.h"
#include "app_rht.h"
#include "app_nvm3.h"
#include "app_display.h"
#include "app_actuator.h"

/***************************************************************************//**
 * @addtogroup app_thermostat
 * @brief  Thermostat application.
 * @details
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Defines

#define THERMOSTAT_BUTTON_EVENT                  (1 << 1)

// -----------------------------------------------------------------------------
// Private variables
static app_timer_t thermostat_timer;
static uint8_t thermostat_connection = 0xff;

static app_thermostat_t app_thermostat;

// -----------------------------------------------------------------------------
// Private function declarations
static void thermostat_data_process(mikroe_shtc3_measurement_data_t data);
static int16_t thermostat_calculate_average(int16_t *data);
static void thermostat_alarm_process(void);
static void thermostat_actuator_process(void);
static void thermostat_timer_cb(app_timer_t *timer, void *data);

static void temperature_notify(void);
static void humidity_notify(void);
static void actuator_notify(void);
static void alarm_status_notify(void);

/***************************************************************************//**
 * Application Init.
 ******************************************************************************/
void thermostat_init(void)
{
  sl_status_t stt;

  app_thermostat.data.actuator_status = 0; // 0 - off
  app_thermostat.data.is_buffer_full = false;
  app_thermostat.data.samples_counter = 0;

  // Initialize and Load configuration from NVM
  app_log_info("Loading parameters from NVM...\r\n");
  stt = nvm3_user_init(&app_thermostat.config);
  if (stt != SL_STATUS_OK) {
    app_log_warning("Load configurations from NVM3 failed\r\n");
  }
  app_log_info("Loaded from NVM. Alarm Enabled = %d\r\n",
               app_thermostat.config.alarm_enabled);
  app_log_info("Loaded from NVM. Mode = %d\r\n",
               app_thermostat.config.mode);
  app_log_info("Loaded from NVM. Setpoint (SV) = %d\r\n",
               app_thermostat.config.setpoint);
  app_log_info("Loaded from NVM. Hysteresis (HYS) = %d\r\n",
               app_thermostat.config.hysteresis);
  app_log_info("Loaded from NVM. Lower threshold = %d\r\n",
               app_thermostat.config.lower_threshold);
  app_log_info("Loaded from NVM. Upper threshold = %d\r\n",
               app_thermostat.config.upper_threshold);
  app_log_info("Loaded from NVM. Measurement Interval = %d\r\n",
               app_thermostat.config.measurement_interval);
  app_log_info("Loaded from NVM. Buzzer volume = %d\r\n",
               app_thermostat.config.buzzer_volume);

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
  buzzer_set_volume(app_thermostat.config.buzzer_volume);
  app_log_info("Buzzer volume is set to = %d\r\n",
               app_thermostat.config.buzzer_volume);

  stt = rht_init();
  if (stt != SL_STATUS_OK) {
    app_log_warning("RHT sensor initialization failed\r\n");
  }
  app_log_info("RHT sensor initialization successfully\r\n");

  // Start timer used for periodic operations
  app_timer_start(&thermostat_timer,
                  app_thermostat.config.measurement_interval * 1000,
                  thermostat_timer_cb,
                  NULL,
                  true);
}

app_thermostat_t thermostat_get_app_data(void)
{
  return app_thermostat;
}

void thermostat_connection_closed_cb(sl_bt_msg_t *evt)
{
  (void)evt;

  thermostat_connection = 0xff;
  app_thermostat.config.notification.temperature = false;
  app_thermostat.config.notification.humidity = false;
  app_thermostat.config.notification.actuator = false;
  app_thermostat.config.notification.alarm_status = false;
}

/***************************************************************************//**
 * Handle bluetooth gatt user write request event.
 ******************************************************************************/
void thermostat_char_write_cb(
  sl_bt_evt_gatt_server_user_write_request_t *data)
{
  sl_status_t response_code = 0;
  sl_status_t stt;

  switch (data->characteristic) {
    case gattdb_mode:
      if (data->value.len == 1) {
        uint8_t value = *(data->value.data);
        stt = nvm3_user_set_mode(value);
        if (stt == SL_STATUS_OK) {
          response_code = SL_STATUS_OK;
          app_thermostat.config.mode = (thermostat_mode_t)value;
          app_log_info("User write request -> Mode: %d\r\n", value);
        } else if (stt == SL_STATUS_INVALID_RANGE) {
          response_code = SL_STATUS_BT_ATT_OUT_OF_RANGE; // The attribute value is out of range
        } else {
          response_code = SL_STATUS_BT_ATT_WRITE_REQUEST_REJECTED; // The requested write operation cannot be fulfilled
        }
      } else {
        response_code = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH; // Invalid Attribute Value Length
      }
      break;

    case gattdb_setpoint:
      if (data->value.len == 2) {
        int16_t value = *((int16_t *)data->value.data);
        stt = nvm3_user_set_setpoint(value);
        if (stt == SL_STATUS_OK) {
          response_code = SL_STATUS_OK;
          app_thermostat.config.setpoint = value;
          app_log_info("User write request -> Setpoint (SV): %d\r\n", value);
        } else if (stt == SL_STATUS_INVALID_RANGE) {
          response_code = SL_STATUS_BT_ATT_OUT_OF_RANGE; // The attribute value is out of range
        } else {
          response_code = SL_STATUS_BT_ATT_WRITE_REQUEST_REJECTED; // The requested write operation cannot be fulfilled
        }
      } else {
        response_code = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH; // Invalid Attribute Value Length
      }
      break;

    case gattdb_hysteresis:
      if (data->value.len == 2) {
        int16_t value = *((int16_t *)data->value.data);
        stt = nvm3_user_set_hysteresis(value);
        if (stt == SL_STATUS_OK) {
          response_code = SL_STATUS_OK;
          app_thermostat.config.hysteresis = value;
          app_log_info("User write request -> Hysteresis (HYS): %d\r\n",
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

    case gattdb_lower_threshold:
      if (data->value.len == 2) {
        int16_t value = *((int16_t *)data->value.data);
        stt = nvm3_user_set_lower_threshold(value);
        if (stt == SL_STATUS_OK) {
          response_code = SL_STATUS_OK;
          app_thermostat.config.lower_threshold = value;
          app_log_info("User write request -> Lower Threshold: %d\r\n",
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

    case gattdb_upper_threshold:
      if (data->value.len == 2) {
        int16_t value = *((int16_t *)data->value.data);
        stt = nvm3_user_set_upper_threshold(value);
        if (stt == SL_STATUS_OK) {
          response_code = SL_STATUS_OK;
          app_thermostat.config.upper_threshold = value;
          app_log_info("User write request -> Upper Threshold: %d\r\n",
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
          app_thermostat.config.measurement_interval = value;
          app_log_info("User write request -> Measurement Interval: %d\r\n",
                       value);
          // Re-Start timer used for periodic operations with new interval
          app_timer_start(&thermostat_timer,
                          app_thermostat.config.measurement_interval * 1000,
                          thermostat_timer_cb,
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

    case gattdb_alarm_enabled:
      if (data->value.len == 1) {
        uint8_t value = *(data->value.data);
        stt = nvm3_user_set_alarm_enabled(value);
        if (stt == SL_STATUS_OK) {
          response_code = SL_STATUS_OK;
          app_thermostat.config.alarm_enabled = value;
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
          app_thermostat.config.buzzer_volume = value;
          app_log_info("User write request -> Buzzer Volume: %d\r\n", value);
          buzzer_set_volume(app_thermostat.config.buzzer_volume);
          app_log_info("Buzzer volume is set to = %d\r\n",
                       app_thermostat.config.buzzer_volume);
        } else if (stt == SL_STATUS_INVALID_RANGE) {
          response_code = SL_STATUS_BT_ATT_OUT_OF_RANGE; // The attribute value is out of range
        } else {
          response_code = SL_STATUS_BT_ATT_WRITE_REQUEST_REJECTED; // The requested write operation cannot be fulfilled
        }
      } else {
        response_code = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH; // Invalid Attribute Value Length
      }
      break;

    default:
      app_assert(false, "Unexpected characteristic\n");
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

/***************************************************************************//**
 * Handle bluetooth gatt user write request event.
 ******************************************************************************/
void thermostat_char_read_cb(
  sl_bt_evt_gatt_server_user_read_request_t *data)
{
  sl_status_t stt;
  uint16_t characteristic_size = 0;
  const uint8_t *characteristic_ptr = NULL;
  uint16_t sent_len;

  // -------------------------------
  // Handle Voice configuration characteristics.
  switch (data->characteristic) {
    case gattdb_mode:
      characteristic_size = sizeof(app_thermostat.config.mode);
      characteristic_ptr = (const uint8_t *) &app_thermostat.config.mode;
      break;

    case gattdb_buzzer_volume:
      characteristic_size = sizeof(app_thermostat.config.buzzer_volume);
      characteristic_ptr =
        (const uint8_t *) &app_thermostat.config.buzzer_volume;
      break;

    case gattdb_setpoint:
      characteristic_size = sizeof(app_thermostat.config.setpoint);
      characteristic_ptr = (const uint8_t *) &app_thermostat.config.setpoint;
      break;

    case gattdb_hysteresis:
      characteristic_size = sizeof(app_thermostat.config.hysteresis);
      characteristic_ptr = (const uint8_t *) &app_thermostat.config.hysteresis;
      break;

    case gattdb_lower_threshold:
      characteristic_size = sizeof(app_thermostat.config.lower_threshold);
      characteristic_ptr =
        (const uint8_t *) &app_thermostat.config.lower_threshold;
      break;

    case gattdb_upper_threshold:
      characteristic_size = sizeof(app_thermostat.config.lower_threshold);
      characteristic_ptr =
        (const uint8_t *) &app_thermostat.config.upper_threshold;
      break;

    case gattdb_alarm_enabled:
      characteristic_size = sizeof(app_thermostat.config.alarm_enabled);
      characteristic_ptr =
        (const uint8_t *) &app_thermostat.config.alarm_enabled;
      break;

    case gattdb_measurement_interval:
      characteristic_size = sizeof(app_thermostat.config.measurement_interval);
      characteristic_ptr =
        (const uint8_t *) &app_thermostat.config.measurement_interval;
      break;

    case gattdb_temperature:
      characteristic_size = sizeof(app_thermostat.data.temperature);
      characteristic_ptr = (const uint8_t *) &app_thermostat.data.temperature;
      break;

    case gattdb_humidity:
      characteristic_size = sizeof(app_thermostat.data.humidity);
      characteristic_ptr = (const uint8_t *) &app_thermostat.data.humidity;
      break;

    case gattdb_actuator:
      characteristic_size = sizeof(app_thermostat.data.actuator_status);
      characteristic_ptr =
        (const uint8_t *) &app_thermostat.data.actuator_status;
      break;

    case gattdb_alarm_status:
      characteristic_size = sizeof(app_thermostat.data.alarm_status);
      characteristic_ptr = (const uint8_t *) &app_thermostat.data.alarm_status;
      break;

    default:
      app_assert(false, "Unexpected characteristic\n");
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

void thermostat_char_config_changed_cb(
  sl_bt_evt_gatt_server_characteristic_status_t *data)
{
  bool enable = sl_bt_gatt_disable != data->client_config_flags;
  void (*notify)(void) = NULL;
  thermostat_connection = data->connection;

  // update notification status
  switch (data->characteristic) {
    case gattdb_temperature:
      app_thermostat.config.notification.temperature = enable;
      notify = &temperature_notify;
      break;
    case gattdb_humidity:
      app_thermostat.config.notification.humidity = enable;
      notify = &humidity_notify;
      break;
    case gattdb_actuator:
      app_thermostat.config.notification.actuator = enable;
      notify = &actuator_notify;
      break;
    case gattdb_alarm_status:
      app_thermostat.config.notification.alarm_status = enable;
      notify = &alarm_status_notify;
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

/***************************************************************************//**
 * Thermostat Application Process External Signal.
 ******************************************************************************/
void thermostat_external_signal_cb(uint32_t extsignals)
{
  if (extsignals & THERMOSTAT_BUTTON_EVENT) {
    if (app_thermostat.config.alarm_enabled) {
      // disable alarm
      app_thermostat.config.alarm_enabled = 0; // 0 - disable
      nvm3_user_set_alarm_enabled(app_thermostat.config.alarm_enabled);
      // deactivate buzzer if it is activating
      if (app_thermostat.data.alarm_status) {
        buzzer_control(0);
      }
    } else {
      // enable alarm
      app_thermostat.config.alarm_enabled = 1; // 1 - enable
      nvm3_user_set_alarm_enabled(app_thermostat.config.alarm_enabled);
      // activate buzzer if alarm is activing
      if (app_thermostat.data.alarm_status) {
        buzzer_control(1);
      }
    }
    app_log_info("Alarm Enabled: %d\r\n", app_thermostat.config.alarm_enabled);
  }
}

/***************************************************************************//**
 * Callback on button change.
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
    if (&sl_button_btn0 == handle) {
      sl_bt_external_signal(THERMOSTAT_BUTTON_EVENT);
    }
  }
}

/***************************************************************************//**
 * Callback on timer period.
 ******************************************************************************/
static void thermostat_timer_cb(app_timer_t *timer, void *data)
{
  (void)data;
  (void)timer;
  app_rht_data_t measurement_data;

  rht_get_data(&measurement_data);

  if ((measurement_data.temperature < (-40))
      || (measurement_data.temperature > 125)
      || (measurement_data.humidity < 0)
      || (measurement_data.humidity > 100)) {
    // error flag
    app_log_info("Invalid data\r\n");
    app_thermostat.data.is_invalid_data = true;
  } else {
    app_thermostat.data.is_invalid_data = false;
    app_log_info(">> Current Temp: %.2f °C Current RH: %.2f %%\n",
                 measurement_data.temperature,
                 measurement_data.humidity);
  }

  thermostat_data_process(measurement_data);
  thermostat_alarm_process();
  thermostat_actuator_process();
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
static int16_t thermostat_calculate_average(int16_t *data)
{
  int32_t sum = 0;
  int16_t min, max;
  uint8_t i, max_count;

  // Avoid Division by zero
  if (!app_thermostat.data.samples_counter) {
    return 0;
  }

  // Calculate average value
  min = max = data[0];
  max_count = app_thermostat.data.is_buffer_full
              ? DATA_BUFFER_SIZE : app_thermostat.data.samples_counter;
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

  return (int16_t) (sum / i);
}

static void thermostat_data_process(app_rht_data_t data)
{
  if (app_thermostat.data.is_invalid_data) {
    return;
  }

  uint8_t index = app_thermostat.data.samples_counter % DATA_BUFFER_SIZE;
  app_thermostat.data.temperature_buffer[index] =
    (int16_t)(data.temperature * 10); // 0.1 C
  app_thermostat.data.humidity_buffer[index] = (int16_t)(data.humidity * 10); // 0.1 %
  app_thermostat.data.samples_counter++;
  if (app_thermostat.data.samples_counter >= DATA_BUFFER_SIZE) {
    app_thermostat.data.is_buffer_full = true;
  }
  app_thermostat.data.temperature = thermostat_calculate_average(
    app_thermostat.data.temperature_buffer);
  app_thermostat.data.humidity = thermostat_calculate_average(
    app_thermostat.data.humidity_buffer);

  app_log_info(">> Average Temp: %.1f °C Average RH: %.1f %%\n",
               (float)app_thermostat.data.temperature / 10,
               (float)app_thermostat.data.humidity / 10);

  if (app_thermostat.config.notification.temperature) {
    temperature_notify();
  }

  if (app_thermostat.config.notification.humidity) {
    humidity_notify();
  }
}

static void thermostat_alarm_process(void)
{
  if ((app_thermostat.data.is_invalid_data)
      || (app_thermostat.data.temperature
          > app_thermostat.config.upper_threshold)
      || (app_thermostat.data.temperature
          < app_thermostat.config.lower_threshold)) {
    // outside => activate
    app_thermostat.data.alarm_status = 1; // 1 - activate
  } else {
    app_thermostat.data.alarm_status = 0; // 0 - deactivate
  }

  // Turn activate/deactivate buzzer if alarm enabled
  if (app_thermostat.config.alarm_enabled) {
    buzzer_control(app_thermostat.data.alarm_status == 1);
  }

  // Send notification if enabled
  if (app_thermostat.config.notification.alarm_status) {
    alarm_status_notify();
  }
}

static void thermostat_actuator_process(void)
{
  if (app_thermostat.data.is_invalid_data) {
    app_thermostat.data.actuator_status = 0;
    goto control;
  }

  if (app_thermostat.data.actuator_status) {
    if ((app_thermostat.config.mode == HEAT)
        && (app_thermostat.data.temperature
            >= app_thermostat.config.setpoint)) {
      // disable actuator
      app_thermostat.data.actuator_status = 0;
    } else if ((app_thermostat.config.mode == COOL)
               && (app_thermostat.data.temperature
                   <= app_thermostat.config.setpoint)) {
      // disable actuator
      app_thermostat.data.actuator_status = 0;
    }
  } else {
    if ((app_thermostat.config.mode == HEAT)
        && (app_thermostat.data.temperature
            < (app_thermostat.config.setpoint
               - app_thermostat.config.hysteresis))) {
      // enable actuator
      app_thermostat.data.actuator_status = 1;
    } else if ((app_thermostat.config.mode == COOL)
               && (app_thermostat.data.temperature
                   > (app_thermostat.config.setpoint
                      + app_thermostat.config.hysteresis))) {
      // enable actuator
      app_thermostat.data.actuator_status = 1;
    }
  }

  control:
  // Turn on/off actuator based on actuator_status
  actuator_control(app_thermostat.data.actuator_status == 1);
  // Send notification if enabled
  if (app_thermostat.config.notification.actuator) {
    actuator_notify();
  }
}

static void temperature_notify(void)
{
  sl_status_t stt;
  stt = sl_bt_gatt_server_send_notification(thermostat_connection,
                                            gattdb_temperature,
                                            sizeof(app_thermostat.data.
                                                   temperature),
                                            (uint8_t *)&app_thermostat.data.temperature);
  app_assert_status(stt);
}

static void humidity_notify(void)
{
  sl_status_t stt;
  stt = sl_bt_gatt_server_send_notification(thermostat_connection,
                                            gattdb_humidity,
                                            sizeof(app_thermostat.data.humidity),
                                            (uint8_t *)&app_thermostat.data.humidity);
  app_assert_status(stt);
}

static void actuator_notify(void)
{
  sl_status_t stt;
  stt = sl_bt_gatt_server_send_notification(thermostat_connection,
                                            gattdb_actuator,
                                            sizeof(app_thermostat.data.
                                                   actuator_status),
                                            (uint8_t *)&app_thermostat.data.actuator_status);
  app_assert_status(stt);
}

static void alarm_status_notify(void)
{
  sl_status_t stt;
  stt = sl_bt_gatt_server_send_notification(thermostat_connection,
                                            gattdb_alarm_status,
                                            sizeof(app_thermostat.data.
                                                   alarm_status),
                                            (uint8_t *)&app_thermostat.data.alarm_status);
  app_assert_status(stt);
}

/** @} (end group app_thermostat) */
