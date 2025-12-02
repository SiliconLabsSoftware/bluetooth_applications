/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
 ******************************************************************************/
#include <display_and_sound.h>
#include "sl_bt_api.h"
#include "sl_main_init.h"
#include "app_assert.h"
#include "app.h"
#include "pulse_oximeter_and_heart_rate_monitor.h"
#include "gatt_db.h"

#define SENSOR_CONTACT_NOT_SUPPORTED             (0x01 << 1)
#define SENSOR_CONTACT_SUPPORTED_NOT_DETECTED    (0x02 << 1)
#define SENSOR_CONTACT_SUPPORTED_DETECTED        (0x03 << 1)

typedef struct {
  bool spo2;
  bool heart_rate;
  bool alarm;
  bool fusion_data;
  bool finger_detection;
}bt_client_notify_enable_t;

typedef enum {
  SPO2_REPORT_FLAG = 0,
  HEART_RATE_REPORT_FLAG = 1,
  ALARM_STATUS_REPORT_FLAG = 2,
  FUSION_DATA_REPORT_FLAG = 3,
  FINGER_DETECTION_REPORT_FLAG = 4,
}send_report_notification_flag_t;

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
static const user_config_t *user_config = NULL;
static const pom_and_hr_monitor_data_t *gpom_and_hr_monitor = NULL;
static bt_client_notify_enable_t client_notify_enable = { 0, 0, 0, 0, 0 };

static void gatt_server_attribute_value_handler(sl_bt_msg_t *evt);
static void gatt_server_characteristic_status_handler(sl_bt_msg_t *evt);
static sl_status_t gatt_server_update_characteristic(void);
static sl_status_t gatt_server_notify_client(void);
static sl_status_t send_report_notification(
  send_report_notification_flag_t flag);

// Application Init.
void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
  sl_status_t sc = pulse_oximeter_and_heart_rate_monitor_init(&user_config,
                                                              &
                                                              gpom_and_hr_monitor);
  app_assert_status(sc);

  display_and_sound_init();
}

// Application Process Action.
void app_process_action(void)
{
  if (app_is_process_required()) {
    /////////////////////////////////////////////////////////////////////////////
    // Put your additional application code here!                              //
    // This is will run each time app_proceed() is called.                     //
    // Do not call blocking functions from here!                               //
    /////////////////////////////////////////////////////////////////////////////
  }
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the default weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160,   // min. adv. interval (milliseconds * 1.6)
        160,   // max. adv. interval (milliseconds * 1.6)
        0,     // adv. duration
        0);    // max. num. adv. events
      app_assert_status(sc);
      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////
    case sl_bt_evt_gatt_server_attribute_value_id:
      gatt_server_attribute_value_handler(evt);
      break;

    // This event occurs when the remote device enabled or disabled the
    // notification.
    case sl_bt_evt_gatt_server_characteristic_status_id:
      gatt_server_characteristic_status_handler(evt);
      break;

    case sl_bt_evt_system_external_signal_id:
      if (evt->data.evt_system_external_signal.extsignals & MEASUREMENT_EVENT) {
        pulse_oximeter_and_heart_rate_monitor_timer_event_handler();
        gatt_server_update_characteristic();
        gatt_server_notify_client();
        display_and_sound_process(gpom_and_hr_monitor, user_config);
      }

      break;
    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

static void gatt_server_attribute_value_handler(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  uint8_t data_recv;
  size_t data_recv_len;

  // -------------------------------
  // This event indicates that the value of an attribute in the local GATT
  // database was changed by a remote GATT client.

  // The value of the gattdb_alarm_enable characteristic was changed.
  if (gattdb_alarm_enable
      == evt->data.evt_gatt_server_attribute_value.attribute) {
    // Read characteristic value.
    sc = sl_bt_gatt_server_read_attribute_value(gattdb_alarm_enable,
                                                0,
                                                gattdb_alarm_enable_len,
                                                &data_recv_len,
                                                &data_recv);
    (void)data_recv_len;
    app_assert_status(sc);

    sc =
      pulse_oximeter_and_heart_rate_monitor_set_user_config(
        SET_USER_CONFIG_FLAG_ALARM_ENABLE,
        data_recv);

    if (sc == SL_STATUS_OK) {
      app_log(
        "Save gattdb_alarm_enable config = %d to the nvm successfully\r\n",
        data_recv);
    } else {
      app_log("Invalid gattdb_alarm_enable config value (%d) !!!\r\n",
              data_recv);
    }
  }

  // The value of the gattdb_buzzer_volume characteristic was changed.
  if (gattdb_buzzer_volume
      == evt->data.evt_gatt_server_attribute_value.attribute) {
    // Read characteristic value.
    sc = sl_bt_gatt_server_read_attribute_value(gattdb_buzzer_volume,
                                                0,
                                                gattdb_buzzer_volume_len,
                                                &data_recv_len,
                                                &data_recv);
    app_assert_status(sc);

    sc =
      pulse_oximeter_and_heart_rate_monitor_set_user_config(
        SET_USER_CONFIG_FLAG_BUZZER_VOLUME,
        data_recv);

    if (sc == SL_STATUS_OK) {
      app_log(
        "Save the gattdb_buzzer_volume config value = %d to the nvm successfully\r\n",
        data_recv);
    } else {
      app_log("Invalid gattdb_buzzer_volume config value (%d) !!!\r\n",
              data_recv);
    }
  }

  // The value of the gattdb_threshold_spo2 characteristic was changed.
  if (gattdb_threshold_spo2
      == evt->data.evt_gatt_server_attribute_value.attribute) {
    // Read characteristic value.
    sc = sl_bt_gatt_server_read_attribute_value(gattdb_threshold_spo2,
                                                0,
                                                gattdb_threshold_spo2_len,
                                                &data_recv_len,
                                                &data_recv);
    app_assert_status(sc);

    sc =
      pulse_oximeter_and_heart_rate_monitor_set_user_config(
        SET_USER_CONFIG_FLAG_THRESHOLD_SPO2,
        data_recv);

    if (sc == SL_STATUS_OK) {
      app_log(
        "Save the gattdb_threshold_spo2 config value = %d to the nvm successfully\r\n",
        data_recv);
    } else {
      app_log("Invalid gattdb_threshold_spo2 config value (%d)!!!\r\n",
              data_recv);
    }
  }

  // The value of the gattdb_threshold_hr_low characteristic was changed.
  if (gattdb_threshold_hr_low
      == evt->data.evt_gatt_server_attribute_value.attribute) {
    // Read characteristic value.
    sc = sl_bt_gatt_server_read_attribute_value(gattdb_threshold_hr_low,
                                                0,
                                                gattdb_threshold_hr_low_len,
                                                &data_recv_len,
                                                &data_recv);
    app_assert_status(sc);

    sc =
      pulse_oximeter_and_heart_rate_monitor_set_user_config(
        SET_USER_CONFIG_FLAG_THRESHOLD_HR_LOW,
        data_recv);

    if (sc == SL_STATUS_OK) {
      app_log(
        "Save the gattdb_threshold_hr_low config value = %d to the nvm successfully\r\n",
        data_recv);
    } else {
      app_log("Invalid gattdb_threshold_hr_low config value (%d)!!!\r\n",
              data_recv);
    }
  }

  // The value of the gattdb_threshold_hr_high characteristic was changed.
  if (gattdb_threshold_hr_high
      == evt->data.evt_gatt_server_attribute_value.attribute) {
    // Read characteristic value.
    sc = sl_bt_gatt_server_read_attribute_value(gattdb_threshold_hr_high,
                                                0,
                                                gattdb_threshold_hr_high_len,
                                                &data_recv_len,
                                                &data_recv);
    app_assert_status(sc);

    sc =
      pulse_oximeter_and_heart_rate_monitor_set_user_config(
        SET_USER_CONFIG_FLAG_THRESHOLD_HR_HIGH,
        data_recv);

    if (sc == SL_STATUS_OK) {
      app_log(
        "Save the gattdb_threshold_hr_high config value = %d to the nvm successfully\r\n",
        data_recv);
    } else {
      app_log("Invalid gattdb_threshold_hr_high config value (%d)!!!\r\n",
              data_recv);
    }
  }
}

static sl_status_t gatt_server_update_characteristic(void)
{
  sl_status_t sc;
  uint8_t hr_send_data[3];
  uint8_t pom_send_data[5];

  // Write attribute in the local GATT database.
  // gattdb_buzzer_volume
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_buzzer_volume,
                                               0,
                                               gattdb_buzzer_volume_len,
                                               &user_config->
                                               buzzer_volume);
  app_assert_status(sc);

  // gattdb_alarm_enable
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_alarm_enable,
                                               0,
                                               gattdb_alarm_enable_len,
                                               &user_config->
                                               alarm_enabled);
  app_assert_status(sc);

  // gattdb_threshold_spo2
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_threshold_spo2,
                                               0,
                                               gattdb_threshold_spo2_len,
                                               &user_config->
                                               threshold_spo2);
  app_assert_status(sc);

  // gattdb_threshold_hr_low
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_threshold_hr_low,
                                               0,
                                               gattdb_threshold_hr_low_len,
                                               &user_config->
                                               threshold_hr_low);
  app_assert_status(sc);

  // gattdb_threshold_hr_high
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_threshold_hr_high,
                                               0,
                                               gattdb_threshold_hr_high_len,
                                               &user_config->
                                               threshold_hr_high);
  app_assert_status(sc);

  // gattdb_plx_continuous_measurement
  pom_send_data[0] = 0x00; // Only present SpO2 and PR
  pom_send_data[1] = gpom_and_hr_monitor->biodata.spo2;
  pom_send_data[2] = 0x00;
  pom_send_data[3] = gpom_and_hr_monitor->biodata.heart_rate;
  pom_send_data[4] = 0x00;

  sc =
    sl_bt_gatt_server_write_attribute_value(gattdb_plx_continuous_measurement,
                                            0,
                                            sizeof(pom_send_data),
                                            (const uint8_t *)pom_send_data);
  app_assert_status(sc);

  // gattdb_heart_rate_measurement
  if (gpom_and_hr_monitor->status.body == FINGER_DETECTED) {
    hr_send_data[0] = SENSOR_CONTACT_SUPPORTED_DETECTED;   // flags - sensor contact
    //   supported and is
    //   detected
  } else {
    hr_send_data[0] = SENSOR_CONTACT_SUPPORTED_NOT_DETECTED;   // flags - sensor
    //   contact supported
    //   and is not
    //   detected
  }

  hr_send_data[1] = gpom_and_hr_monitor->biodata.heart_rate;
  hr_send_data[2] = 0x00;

  sc = sl_bt_gatt_server_write_attribute_value(gattdb_heart_rate_measurement,
                                               0,
                                               sizeof(hr_send_data),
                                               (const uint8_t *)hr_send_data);
  app_assert_status(sc);

  // gattdb_alarm_status
  bool alarm_stt = (gpom_and_hr_monitor->status.spo2_alarm)
                   || (gpom_and_hr_monitor->status.low_heart_rate_alarm)
                   || (gpom_and_hr_monitor->status.high_heart_rate_alarm);
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_alarm_status,
                                               0,
                                               sizeof(alarm_stt),
                                               (const uint8_t *)&alarm_stt);
  app_assert_status(sc);

  // gattdb_sensor_fusion_data
  uint8_t fusion_buffer[gattdb_sensor_fusion_data_len] =
  { gpom_and_hr_monitor->biodata.heart_rate, gpom_and_hr_monitor->biodata.spo2,
    gpom_and_hr_monitor->biodata.confidence };
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_sensor_fusion_data,
                                               0,
                                               gattdb_sensor_fusion_data_len,
                                               (const uint8_t *)fusion_buffer);

  app_assert_status(sc);

  // gattdb_finger_detection_status
  int8_t finger_detection_buffer[gattdb_finger_detection_status_len] =
  { (int8_t)gpom_and_hr_monitor->status.body,
    (int8_t)gpom_and_hr_monitor->status.body_ext };
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_finger_detection_status,
                                               0,
                                               gattdb_finger_detection_status_len,
                                               (const uint8_t *)
                                               finger_detection_buffer);

  app_assert_status(sc);

  return SL_STATUS_OK;
}

static sl_status_t gatt_server_notify_client(void)
{
  static pom_and_hr_monitor_data_t temp_pom_and_hr_monitor;
  sl_status_t sc;
  bool alarm_status_send = false;
  bool fusion_data_send = false;

  if ((temp_pom_and_hr_monitor.biodata.spo2
       != gpom_and_hr_monitor->biodata.spo2)
      || (temp_pom_and_hr_monitor.biodata.heart_rate
          != gpom_and_hr_monitor->biodata.heart_rate)
      ) {
    temp_pom_and_hr_monitor.biodata.spo2 = gpom_and_hr_monitor->biodata.spo2;
    temp_pom_and_hr_monitor.biodata.heart_rate =
      gpom_and_hr_monitor->biodata.heart_rate;
    fusion_data_send = true;

    if (client_notify_enable.spo2) {
      sc = send_report_notification(SPO2_REPORT_FLAG);
      app_log_status_error(sc);
    }

    if (client_notify_enable.heart_rate) {
      sc = send_report_notification(HEART_RATE_REPORT_FLAG);
      app_log_status_error(sc);
    }
  }

  if (temp_pom_and_hr_monitor.status.spo2_alarm
      != gpom_and_hr_monitor->status.spo2_alarm) {
    temp_pom_and_hr_monitor.status.spo2_alarm =
      gpom_and_hr_monitor->status.spo2_alarm;
    alarm_status_send = true;
  }
  if (temp_pom_and_hr_monitor.status.low_heart_rate_alarm
      != gpom_and_hr_monitor->status.low_heart_rate_alarm) {
    temp_pom_and_hr_monitor.status.low_heart_rate_alarm =
      gpom_and_hr_monitor->status.low_heart_rate_alarm;
    alarm_status_send = true;
  }
  if (temp_pom_and_hr_monitor.status.high_heart_rate_alarm
      != gpom_and_hr_monitor->status.high_heart_rate_alarm) {
    temp_pom_and_hr_monitor.status.high_heart_rate_alarm =
      gpom_and_hr_monitor->status.high_heart_rate_alarm;
    alarm_status_send = true;
  }
  if (alarm_status_send && client_notify_enable.alarm) {
    sc = send_report_notification(ALARM_STATUS_REPORT_FLAG);
    app_log_status_error(sc);
  }

  if (temp_pom_and_hr_monitor.biodata.confidence
      != gpom_and_hr_monitor->biodata.confidence) {
    temp_pom_and_hr_monitor.biodata.confidence =
      gpom_and_hr_monitor->biodata.confidence;
    fusion_data_send = true;
  }
  if (fusion_data_send && client_notify_enable.fusion_data) {
    sc = send_report_notification(FUSION_DATA_REPORT_FLAG);
    app_log_status_error(sc);
  }

  if ((temp_pom_and_hr_monitor.status.body
       != gpom_and_hr_monitor->status.body)
      || (temp_pom_and_hr_monitor.status.body_ext
          != gpom_and_hr_monitor->status.body_ext)
      ) {
    temp_pom_and_hr_monitor.status.body = gpom_and_hr_monitor->status.body;
    temp_pom_and_hr_monitor.status.body_ext =
      gpom_and_hr_monitor->status.body_ext;

    if (client_notify_enable.finger_detection) {
      sc = send_report_notification(FINGER_DETECTION_REPORT_FLAG);
      app_log_status_error(sc);
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t send_report_notification(
  send_report_notification_flag_t flag)
{
  sl_status_t sc = SL_STATUS_OK;
  uint16_t attribute;
  uint8_t data_read[20];
  size_t data_read_len;
  uint8_t  length;

  switch (flag) {
    case SPO2_REPORT_FLAG:
      attribute = gattdb_plx_continuous_measurement;
      length = gattdb_plx_continuous_measurement_len;
      break;

    case HEART_RATE_REPORT_FLAG:
      attribute = gattdb_heart_rate_measurement;
      length = gattdb_heart_rate_measurement_len;
      break;

    case ALARM_STATUS_REPORT_FLAG:
      attribute = gattdb_alarm_status;
      length = gattdb_alarm_status_len;
      break;

    case FUSION_DATA_REPORT_FLAG:
      attribute = gattdb_sensor_fusion_data;
      length = gattdb_sensor_fusion_data_len;
      break;

    case FINGER_DETECTION_REPORT_FLAG:
      attribute = gattdb_finger_detection_status;
      length = gattdb_finger_detection_status_len;
      break;

    default:
      sc = SL_STATUS_INVALID_PARAMETER;
      break;
  }

  if (sc != SL_STATUS_OK) {
    return sc;
  }

  sc = sl_bt_gatt_server_read_attribute_value(attribute,
                                              0,
                                              length,
                                              &data_read_len,
                                              data_read);
  app_assert_status(sc);

  // Send characteristic notification
  sc = sl_bt_gatt_server_notify_all(attribute,
                                    data_read_len,
                                    data_read);
  app_assert_status(sc);
  app_log("sent report: attribute = %d, length = %d\r\n",
          attribute,
          data_read_len);

  return SL_STATUS_OK;
}

static void gatt_server_characteristic_status_handler(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  if (gattdb_plx_continuous_measurement
      == evt->data.evt_gatt_server_characteristic_status.characteristic) {
    if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
        & sl_bt_gatt_notification) {
      app_log("SpO2 notification enabled" APP_LOG_NL);
      client_notify_enable.spo2 = true;
      sc = send_report_notification(SPO2_REPORT_FLAG);
      app_log_status_error(sc);
    } else {
      app_log("SpO2 notification disable" APP_LOG_NL);
      client_notify_enable.spo2 = false;
    }
  }

  if (gattdb_heart_rate_measurement
      == evt->data.evt_gatt_server_characteristic_status.characteristic) {
    if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
        & sl_bt_gatt_notification) {
      app_log("Heart rate notification enabled" APP_LOG_NL);
      client_notify_enable.heart_rate = true;
      sc = send_report_notification(HEART_RATE_REPORT_FLAG);
      app_log_status_error(sc);
    } else {
      app_log("Heart rate notification disable" APP_LOG_NL);
      client_notify_enable.heart_rate = false;
    }
  }

  if (gattdb_alarm_status
      == evt->data.evt_gatt_server_characteristic_status.characteristic) {
    if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
        & sl_bt_gatt_notification) {
      app_log("Alarm status notification enabled" APP_LOG_NL);
      client_notify_enable.alarm = true;
      sc = send_report_notification(ALARM_STATUS_REPORT_FLAG);
      app_log_status_error(sc);
    } else {
      app_log("Alarm status notification disable" APP_LOG_NL);
      client_notify_enable.alarm = false;
    }
  }

  if (gattdb_sensor_fusion_data
      == evt->data.evt_gatt_server_characteristic_status.characteristic) {
    if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
        & sl_bt_gatt_notification) {
      app_log("Fusion data notification enabled" APP_LOG_NL);
      client_notify_enable.fusion_data = true;
      sc = send_report_notification(FUSION_DATA_REPORT_FLAG);
      app_log_status_error(sc);
    } else {
      app_log("Fusion data notification disable" APP_LOG_NL);
      client_notify_enable.fusion_data = false;
    }
  }

  if (gattdb_finger_detection_status
      == evt->data.evt_gatt_server_characteristic_status.characteristic) {
    if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
        & sl_bt_gatt_notification) {
      app_log("Finger detection status notification enabled" APP_LOG_NL);
      client_notify_enable.finger_detection = true;
      sc = send_report_notification(FINGER_DETECTION_REPORT_FLAG);
      app_log_status_error(sc);
    } else {
      app_log("Finger detection status notification disable" APP_LOG_NL);
      client_notify_enable.finger_detection = false;
    }
  }
}
