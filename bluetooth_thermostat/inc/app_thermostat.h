/***************************************************************************//**
 * @file app_thermostat.h
 * @brief Thermostat application header
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

#ifndef APP_THERMOSTAT_H_
#define APP_THERMOSTAT_H_

#include <stdint.h>
#include "sl_bt_api.h"

#define DATA_BUFFER_SIZE                         5

// -----------------------------------------------------------------------------
// Enum
typedef enum {
  HEAT = 0,
  COOL =1
} thermostat_mode_t;

/***************************************************************************//**
 * @brief
 *    Typedef for holding thermostat notification config.
 ******************************************************************************/
typedef struct {
  bool temperature;
  bool humidity;
  bool alarm_status;
  bool actuator;
} thermostat_notification_t;

/***************************************************************************//**
 * @brief
 *    Typedef for holding thermostat configuration parameters.
 ******************************************************************************/
typedef struct {
  uint8_t mode;
  int16_t setpoint;
  int16_t hysteresis;
  int16_t lower_threshold;
  int16_t upper_threshold;
  uint8_t alarm_enabled;
  uint8_t measurement_interval;
  uint8_t buzzer_volume;
  thermostat_notification_t notification;
} thermostat_config_t;

/***************************************************************************//**
 * @brief
 *    Typedef for holding measurement data.
 ******************************************************************************/
typedef struct {
  int16_t temperature;
  int16_t humidity;
  bool is_invalid_data;
  uint8_t alarm_status;
  uint8_t actuator_status;
  int16_t temperature_buffer[DATA_BUFFER_SIZE];
  int16_t humidity_buffer[DATA_BUFFER_SIZE];
  uint32_t samples_counter;
  bool is_buffer_full;
} thermostat_data_t;

/***************************************************************************//**
 * @brief
 *    Typedef for holding thermostat application.
 ******************************************************************************/
typedef struct {
  thermostat_data_t data;
  thermostat_config_t config;
} app_thermostat_t;

/***************************************************************************//**
 * @brief
 *  Initialize the thermostat application.
 *
 ******************************************************************************/
void thermostat_init(void);

/***************************************************************************//**
 * @brief
 *  Initialize the thermostat application.
 *
 ******************************************************************************/
app_thermostat_t thermostat_get_app_data(void);

/***************************************************************************//**
 * @brief
 *    Handle bluetooth gatt connection closed event.
 *
 * @param[in] data
 *    Data structure of the user_write_request event
 *
 ******************************************************************************/
void thermostat_connection_closed_cb(sl_bt_msg_t *evt);

/***************************************************************************//**
 * @brief
 *    Handle bluetooth gatt user write request event.
 *
 * @param[in] data
 *    Data structure of the user_write_request event
 *
 ******************************************************************************/
void thermostat_char_write_cb(
  sl_bt_evt_gatt_server_user_write_request_t *data);

/***************************************************************************//**
 * @brief
 *    Handle bluetooth gatt user read request event.
 *
 * @param[in] data
 *    Data structure of the user_read_request event
 *
 ******************************************************************************/
void thermostat_char_read_cb(
  sl_bt_evt_gatt_server_user_read_request_t *data);

/*******************************************************************************
 * @brief
 *   Function to handle characteristic status event.
 *
 * @param[in] data
 *   Data structure of the characteristic_status event
 *
 * @return
 *   None
 ******************************************************************************/
void thermostat_char_config_changed_cb(
  sl_bt_evt_gatt_server_characteristic_status_t *data);

/***************************************************************************//**
 * @brief
 *    Handle bluetooth event external signal.
 *
 * @param[in] extsignals
 *    Event flags.
 *
 ******************************************************************************/
void thermostat_external_signal_cb(uint32_t extsignals);

#endif // APP_THERMOSTAT_H_
