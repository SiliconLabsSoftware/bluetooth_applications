/***************************************************************************//**
 * @file app_air_quality.h
 * @brief Define driver structures and APIs for the air_quality_app.c
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
#ifndef AIR_QUALITY_APP_H
#define AIR_QUALITY_APP_H

#include "sl_bt_api.h"

#define DATA_BUFFER_SIZE                    5

// External Events
#define AIR_QUALITY_MONITOR_BUTTON_EVENT    1

/***************************************************************************//**
 * @brief
 *    Typedef for holding air quality notification config.
 ******************************************************************************/
typedef struct {
  bool co2;
  bool tvoc;
  bool alarm_status;
  bool aqi;
} air_quality_notification_t;

/***************************************************************************//**
 * @brief
 *    Typedef for holding air quality configuration parameters.
 ******************************************************************************/
typedef struct {
  uint16_t co2_threshold;
  uint16_t tvoc_threshold;
  uint8_t alarm_enabled;
  uint8_t measurement_interval;
  uint8_t buzzer_volume;
  air_quality_notification_t notification;
} air_quality_config_t;

/***************************************************************************//**
 * @brief
 *    Typedef for holding measurement data.
 ******************************************************************************/
typedef struct {
  uint16_t co2;
  uint16_t tvoc;
  bool is_invalid_data;
  uint8_t alarm_status;
  uint8_t aqi;
  uint16_t co2_buffer[DATA_BUFFER_SIZE];
  uint16_t tvoc_buffer[DATA_BUFFER_SIZE];
  uint32_t samples_counter;
  bool is_buffer_full;
} air_quality_data_t;

/***************************************************************************//**
 * @brief
 *    Typedef for holding air quality application.
 ******************************************************************************/
typedef struct {
  air_quality_data_t data;
  air_quality_config_t config;
} app_air_quality_t;

/***************************************************************************//**
 * @addtogroup air_quality_app
 * @brief air_quality_app interface.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *    Return codes for the status of air quality index function.
 ******************************************************************************/
typedef enum air_quality_index {
  EXCELLENT   = 1, ///< The air inside is as fresh as the air outside.
  FINE        = 2, ///< The air quality inside remains at harmless levels.
  MODERATE    = 3, ///< The air quality inside has reached conspicuous levels.
  POOR        = 4, ///< The air quality inside has reached precarious levels.
  VERY_POOR   = 5, ///< The air quality inside has reached unacceptable levels.
  SEVERE      = 6  ///< The air quality inside has exceeded maximum workplace
                   ///<   concentration values.
} air_quality_index_t;

/*********************************************************************&*****//**
 * @brief
 *   Initialize the AIR QUALITY application.
 *
 * @return
 *   None
 ********************************************************************&*********/
void air_quality_init(void);

/*******************************************************************************
 * @brief
 *   Function to get air quality data
 *
 * @return
 *   Data to get
 ******************************************************************************/
air_quality_data_t air_quality_get_data(void);

/***************************************************************************//**
 * @brief
 *    Handle bluetooth event external signal.
 *
 * @param[in] extsignals
 *    Event flags.
 *
 ******************************************************************************/
void air_quality_external_signal_cb(uint32_t event_flags);

/*******************************************************************************
 * @brief
 *   Function to handle write data.
 *
 * @param[in] evt
 *   Gecko event
 *
 * @return
 *   None
 ******************************************************************************/
void air_quality_user_write_cb(
  sl_bt_evt_gatt_server_user_write_request_t *data);

/*******************************************************************************
 * @brief
 *   Function to handle read event.
 *
 * @param[in] evt
 *   Gecko event
 *
 * @return
 *   None
 ******************************************************************************/
void air_quality_user_read_cb(
  sl_bt_evt_gatt_server_user_read_request_t *data);

/*******************************************************************************
 * @brief
 *   Function to handle characteristic status event.
 *
 * @param[in] data
 *   characteristic_status event
 *
 * @return
 *   None
 ******************************************************************************/
void air_quality_characteristic_status_cb(
  sl_bt_evt_gatt_server_characteristic_status_t *data);

/*******************************************************************************
 * @brief
 *   Function to handle connection closed event.
 *
 * @param[in] evt
 *   Gecko event
 *
 * @return
 *   None
 ******************************************************************************/
void air_quality_connection_closed_cb(sl_bt_msg_t *evt);

/** @} */

#endif //AIR_QUALITY_APP_H
