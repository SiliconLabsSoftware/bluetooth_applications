/***************************************************************************//**
 * @file pulse_oximeter_and_heart_rate_monitor.h
 * @brief pulse_oximeter_and_heart_rate_monitor application interface
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
#ifndef PULSE_OXIMETER_AND_HEART_RATE_MONITOR_H
#define PULSE_OXIMETER_AND_HEART_RATE_MONITOR_H
#include "sl_component_catalog.h"
#include "stdbool.h"
#include "stdint.h"
#include "sl_status.h"

#define MEASUREMENT_EVENT       (0x01)

typedef enum {
  SET_USER_CONFIG_FLAG_ALARM_ENABLE = 0,
  SET_USER_CONFIG_FLAG_BUZZER_VOLUME = 1,
  SET_USER_CONFIG_FLAG_THRESHOLD_SPO2 = 2,
  SET_USER_CONFIG_FLAG_THRESHOLD_HR_LOW = 3,
  SET_USER_CONFIG_FLAG_THRESHOLD_HR_HIGH = 4,
}set_user_config_flag_t;

typedef struct {
  bool alarm_enabled;
  uint8_t buzzer_volume;
  uint8_t threshold_spo2;
  uint8_t threshold_hr_low;
  uint8_t threshold_hr_high;
}user_config_t;

typedef enum {
  NO_OBJECT_DETECTED = 0,
  OBJECT_DETECTED = 1,
  OBJECT_OTHER_THAN_FINGER_DETECTED = 2,
  FINGER_DETECTED = 3,
}body_status_t;

typedef enum {
  EXT_SUCCESS = 0,
  EXT_NOT_READY = 1,
  EXT_OBJECT_DETECTED = -1,
  EXT_EXCESSIVE_SENSOR_DEVICE_MOTION = -2,
  EXT_NO_OBJECT_DETECTED = -3,
  EXT_PRESSING_TOO_HARD = -4,
  EXT_OBJECT_OTHER_THAN_FINGER_DETECTED = -5,
  EXT_EXCESSIVE_FINGER_MOTION = -6,
}body_ext_status_t;

typedef struct {
  struct {
    body_status_t body;
    body_ext_status_t body_ext;
    bool spo2_alarm;
    bool low_heart_rate_alarm;
    bool high_heart_rate_alarm;
  } status;

  struct {
    uint8_t spo2;
    uint8_t heart_rate;
    uint8_t confidence;
  } biodata;
}pom_and_hr_monitor_data_t;

sl_status_t pulse_oximeter_and_heart_rate_monitor_init(
  const user_config_t **config,
  const pom_and_hr_monitor_data_t **data);
void pulse_oximeter_and_heart_rate_monitor_timer_event_handler(void);
sl_status_t pulse_oximeter_and_heart_rate_monitor_set_user_config(
  set_user_config_flag_t config_flag,
  uint8_t value);

#endif // PULSE_OXIMETER_AND_HEART_RATE_MONITOR_H
