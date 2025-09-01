/***************************************************************************//**
 * @file app.c
 * @brief CS presence detection example application logic
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
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "sl_component_catalog.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"

#include "ble_peer_manager_common.h"
#include "ble_peer_manager_connections.h"
#include "ble_peer_manager_central.h"
#include "ble_peer_manager_filter.h"

#include "sl_common.h"
#include "app_assert.h"
#include "app.h"
#include "app_log.h"
#include "app_config.h"
#include "cs_antenna.h"
#include "cs_initiator.h"
#include "cs_initiator_client.h"
#include "cs_initiator_config.h"
#include "cs_presence_detection.h"

#ifdef SL_CATALOG_CS_INITIATOR_CLI_PRESENT
#include "cs_initiator_cli.h"
#endif // SL_CATALOG_CS_INITIATOR_CLI_PRESENT

#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

#include "cs_ras_client.h"

// -----------------------------------------------------------------------------
// Enums, structs, typedef

// Measurement structure
typedef struct {
  float distance_filtered;
  float distance_raw;
  float likeliness;
  float distance_estimate_rssi;
  float velocity;
  float bit_error_rate;
} cs_measurement_data_t;

// CS initiator instance
typedef struct {
  uint8_t conn_handle;
  uint32_t measurement_cnt;
  uint32_t ranging_counter;
  cs_measurement_data_t measurement_mainmode;
  cs_measurement_data_t measurement_submode;
  cs_intermediate_result_t measurement_progress;
  bool measurement_arrived;
  bool measurement_progress_changed;
  bool read_remote_capabilities;
  uint8_t number_of_measurements;
} cs_initiator_instances_t;

// -----------------------------------------------------------------------------
// Static function declarations

static uint8_t get_algo_mode(void);
static void cs_on_result(const uint8_t conn_handle,
                         const uint16_t ranging_counter,
                         const uint8_t *result,
                         const cs_result_session_data_t *result_data,
                         const cs_ranging_data_t *ranging_data,
                         const void *user_data);
static void cs_on_intermediate_result(
  const cs_intermediate_result_t *intermediate_result,
  const void *user_data);
static void cs_on_error(uint8_t conn_handle,
                        cs_error_event_t err_evt,
                        sl_status_t sc);
static sl_status_t get_instance_number(uint8_t conn_handle,
                                       uint8_t *instance_num);
static sl_status_t create_new_initiator_instance(uint8_t conn_handle);

static cs_initiator_config_t initiator_config = INITIATOR_CONFIG_DEFAULT;
static rtl_config_t rtl_config = RTL_CONFIG_DEFAULT;
static uint8_t num_reflector_connections = 0u;
static cs_initiator_instances_t cs_initiator_instances[
  CS_INITIATOR_MAX_CONNECTIONS];

/******************************************************************************
 * Application Init
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  sl_status_t sc = SL_STATUS_OK;

  // initialize initiator instances
  for (uint32_t i = 0u; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    cs_initiator_instances[i].conn_handle = SL_BT_INVALID_CONNECTION_HANDLE;
    cs_initiator_instances[i].measurement_cnt = 0u;
    cs_initiator_instances[i].ranging_counter = 0u;
    memset(&cs_initiator_instances[i].measurement_mainmode, 0u,
           sizeof(cs_measurement_data_t));
    memset(&cs_initiator_instances[i].measurement_submode, 0u,
           sizeof(cs_measurement_data_t));
    memset(&cs_initiator_instances[i].measurement_progress, 0u,
           sizeof(cs_intermediate_result_t));
    cs_initiator_instances[i].measurement_arrived = false;
    cs_initiator_instances[i].measurement_progress_changed = false;
    cs_initiator_instances[i].read_remote_capabilities = false;
    cs_initiator_instances[i].number_of_measurements = 0u;
  }

  // Set configuration parameters
  rtl_config.algo_mode = get_algo_mode();
  cs_initiator_apply_channel_map_preset(initiator_config.channel_map_preset,
                                        initiator_config.channel_map.data);

  if ((initiator_config.cs_main_mode == sl_bt_cs_mode_pbr)
      && (initiator_config.cs_sub_mode == sl_bt_cs_mode_rtt)) {
    // Set mode and submode. Currently, only main mode = pbr and submode = rtt is supported
    initiator_config.min_main_mode_steps =
      CS_INITIATOR_MIXED_MODE_MAIN_MODE_STEPS;
    initiator_config.max_main_mode_steps =
      CS_INITIATOR_MIXED_MODE_MAIN_MODE_STEPS;
    initiator_config.channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH;
  }

  app_log("Channel Sounding Based Presence Detection\r\n");

  sc = cs_presence_detection_init();
  app_assert_status(sc);
}

/******************************************************************************
 * Application Process Action
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  for (uint8_t i = 0u; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    if (cs_initiator_instances[i].measurement_arrived) {
      cs_initiator_instances[i].measurement_arrived = false;
      cs_presence_detection_processing(
        cs_initiator_instances[i].measurement_mainmode.distance_filtered);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

// -----------------------------------------------------------------------------
// Static function definitions

/******************************************************************************
 * Return runtime configurable value for object tracking mode
 *****************************************************************************/
#if (SL_SIMPLE_BUTTON_COUNT > 1)
#if CS_INITIATOR_DEFAULT_ALGO_MODE == SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST
#define CS_INITIATOR_ALTERNATIVE_ALGO_MODE \
  SL_RTL_CS_ALGO_MODE_STATIC_HIGH_ACCURACY
#else
#define CS_INITIATOR_ALTERNATIVE_ALGO_MODE SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST
#endif
static uint8_t get_algo_mode(void)
{
  if (sl_button_get_state(SL_SIMPLE_BUTTON_INSTANCE(1))
      == SL_SIMPLE_BUTTON_PRESSED) {
    return CS_INITIATOR_ALTERNATIVE_ALGO_MODE;
  }
  return CS_INITIATOR_DEFAULT_ALGO_MODE;
}

#else
static uint8_t get_algo_mode(void)
{
  return CS_INITIATOR_DEFAULT_ALGO_MODE;
}

#endif

/******************************************************************************
 * Get instance number based on connection handle
 *****************************************************************************/
static sl_status_t get_instance_number(uint8_t conn_handle,
                                       uint8_t *instance_num)
{
  for (uint8_t i = 0u; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    if (cs_initiator_instances[i].conn_handle == conn_handle) {
      *instance_num = i;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_FAIL;
}

/******************************************************************************
 * Extract measurement results
 *****************************************************************************/
static void cs_on_result(const uint8_t conn_handle,
                         const uint16_t ranging_counter,
                         const uint8_t *result,
                         const cs_result_session_data_t *result_data,
                         const cs_ranging_data_t *ranging_data,
                         const void *user_data)
{
  (void)ranging_data;
  (void)user_data;
  uint8_t initiator_num;

  if (result != NULL) {
    sl_status_t sc = get_instance_number(conn_handle, &initiator_num);
    app_assert_status(sc);

    sc = cs_result_extract_field((cs_result_session_data_t *)result_data,
                                 CS_RESULT_FIELD_DISTANCE_MAINMODE,
                                 (uint8_t *)result,
                                 (uint8_t *)&cs_initiator_instances[
                                   initiator_num].measurement_mainmode.distance_filtered);
    app_assert_status(sc);

    if (initiator_config.cs_sub_mode != sl_bt_cs_submode_disabled) {
      sc = cs_result_extract_field((cs_result_session_data_t *)result_data,
                                   CS_RESULT_FIELD_DISTANCE_SUBMODE,
                                   (uint8_t *)result,
                                   (uint8_t *)&cs_initiator_instances[
                                     initiator_num].measurement_submode.distance_filtered);
      app_assert_status(sc);
    }

    sc = cs_result_extract_field((cs_result_session_data_t *)result_data,
                                 CS_RESULT_FIELD_DISTANCE_RAW_MAINMODE,
                                 (uint8_t *)result,
                                 (uint8_t *)&cs_initiator_instances[
                                   initiator_num].measurement_mainmode.distance_raw);
    app_assert_status(sc);

    if (initiator_config.cs_sub_mode != sl_bt_cs_submode_disabled) {
      sc = cs_result_extract_field((cs_result_session_data_t *)result_data,
                                   CS_RESULT_FIELD_DISTANCE_RAW_SUBMODE,
                                   (uint8_t *)result,
                                   (uint8_t *)&cs_initiator_instances[
                                     initiator_num].measurement_submode.distance_raw);
      app_assert_status(sc);
    }

    sc = cs_result_extract_field((cs_result_session_data_t *)result_data,
                                 CS_RESULT_FIELD_LIKELINESS_MAINMODE,
                                 (uint8_t *)result,
                                 (uint8_t *)&cs_initiator_instances[
                                   initiator_num].measurement_mainmode.likeliness);
    app_assert_status(sc);

    if (initiator_config.cs_sub_mode != sl_bt_cs_submode_disabled) {
      sc = cs_result_extract_field((cs_result_session_data_t *)result_data,
                                   CS_RESULT_FIELD_LIKELINESS_SUBMODE,
                                   (uint8_t *)result,
                                   (uint8_t *)&cs_initiator_instances[
                                     initiator_num].measurement_submode.likeliness);
      app_assert_status(sc);
    }

    if ((rtl_config.algo_mode == SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST)
        && (initiator_config.cs_main_mode == sl_bt_cs_mode_pbr)
        && ((initiator_config.channel_map_preset == CS_CHANNEL_MAP_PRESET_HIGH)
            || (initiator_config.channel_map_preset
                == CS_CHANNEL_MAP_PRESET_MEDIUM))) {
      sc = cs_result_extract_field((cs_result_session_data_t *)result_data,
                                   CS_RESULT_FIELD_VELOCITY_MAINMODE,
                                   (uint8_t *)result,
                                   (uint8_t *)&cs_initiator_instances[
                                     initiator_num].measurement_mainmode.velocity);
      app_assert_status(sc);
    }

    // BER is only for RTT
    if (initiator_config.cs_main_mode == sl_bt_cs_mode_rtt) {
      sc = cs_result_extract_field((cs_result_session_data_t *)result_data,
                                   CS_RESULT_FIELD_BIT_ERROR_RATE,
                                   (uint8_t *)result,
                                   (uint8_t *)&cs_initiator_instances[
                                     initiator_num].measurement_mainmode.bit_error_rate);
      app_assert_status(sc);
    }

    // Extract RSSI distance always
    sc = cs_result_extract_field((cs_result_session_data_t *)result_data,
                                 CS_RESULT_FIELD_DISTANCE_RSSI,
                                 (uint8_t *)result,
                                 (uint8_t *)&cs_initiator_instances[
                                   initiator_num].measurement_mainmode.distance_estimate_rssi);
    app_assert_status(sc);
    cs_initiator_instances[initiator_num].measurement_arrived = true;
    cs_initiator_instances[initiator_num].measurement_cnt++;
    cs_initiator_instances[initiator_num].ranging_counter = ranging_counter;
  } else {
    app_assert_status(SL_STATUS_FAIL);
  }
}

/******************************************************************************
 * Extract intermediate results between measurement results
 * Note: only called when stationary object tracking used
 *****************************************************************************/
static void cs_on_intermediate_result(
  const cs_intermediate_result_t *intermediate_result,
  const void *user_data)
{
  (void) user_data;
  uint8_t instance_num;
  if (intermediate_result != NULL) {
    sl_status_t sc = get_instance_number(intermediate_result->connection,
                                         &instance_num);
    app_assert_status(sc);

    memcpy(&cs_initiator_instances[instance_num].measurement_progress,
           intermediate_result,
           sizeof(cs_intermediate_result_t));
    cs_initiator_instances[instance_num].measurement_progress_changed = true;
  }
}

/******************************************************************************
 * Create new initiator instance
 *****************************************************************************/
static sl_status_t create_new_initiator_instance(uint8_t conn_handle)
{
  sl_status_t sc;
  cs_intermediate_result_t measurement_progress;
  // Check if we can accept one more reflector connection
  if (num_reflector_connections >= CS_INITIATOR_MAX_CONNECTIONS) {
    return SL_STATUS_FULL;
  }
  // Store the new initiator instance
  for (uint32_t i = 0u; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    if (cs_initiator_instances[i].conn_handle
        == SL_BT_INVALID_CONNECTION_HANDLE) {
      cs_initiator_instances[i].conn_handle = conn_handle;
      cs_initiator_instances[i].measurement_cnt = 0u;
      memset(&cs_initiator_instances[i].measurement_mainmode, 0u,
             sizeof(cs_measurement_data_t));
      memset(&cs_initiator_instances[i].measurement_submode, 0u,
             sizeof(cs_measurement_data_t));
      memset(&cs_initiator_instances[i].measurement_progress, 0u,
             sizeof(measurement_progress));
      num_reflector_connections++;
      break;
    }
  }

  sc = cs_initiator_create(conn_handle,
                           &initiator_config,
                           &rtl_config,
                           cs_on_result,
                           cs_on_intermediate_result,
                           cs_on_error,
                           NULL);
  if (sc != SL_STATUS_OK) {
    (void)ble_peer_manager_central_close_connection(conn_handle);
  }
  return sc;
}

/******************************************************************************
 * Delete initiator instance
 *****************************************************************************/
static void delete_initiator_instance(uint8_t conn_handle)
{
  for (uint32_t i = 0u; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    if (cs_initiator_instances[i].conn_handle == conn_handle) {
      cs_initiator_instances[i].conn_handle = SL_BT_INVALID_CONNECTION_HANDLE;
      cs_initiator_instances[i].measurement_cnt = 0u;
      memset(&cs_initiator_instances[i].measurement_mainmode, 0u,
             sizeof(cs_measurement_data_t));
      memset(&cs_initiator_instances[i].measurement_submode, 0u,
             sizeof(cs_measurement_data_t));
      memset(&cs_initiator_instances[i].measurement_progress, 0u,
             sizeof(cs_intermediate_result_t));
      cs_initiator_instances[i].measurement_arrived = false;
      cs_initiator_instances[i].measurement_progress_changed = false;
      cs_initiator_instances[i].read_remote_capabilities = false;
      num_reflector_connections--;
      break;
    }
  }
}

/******************************************************************************
 * CS error handler
 *****************************************************************************/
static void cs_on_error(uint8_t conn_handle,
                        cs_error_event_t err_evt,
                        sl_status_t sc)
{
  (void)sc;

  switch (err_evt) {
    // Assert
    case CS_ERROR_EVENT_CS_PROCEDURE_STOP_TIMER_FAILED:
    case CS_ERROR_EVENT_CS_PROCEDURE_UNEXPECTED_DATA:
      break;

    // Discard
    case CS_ERROR_EVENT_RTL_PROCESS_ERROR:
      break;

    case CS_ERROR_EVENT_INITIATOR_FAILED_TO_SET_INTERVALS:
      break;
    // Antenna usage not supported
    case CS_ERROR_EVENT_INITIATOR_PBR_ANTENNA_USAGE_NOT_SUPPORTED:
      break;
    case CS_ERROR_EVENT_INITIATOR_RTT_ANTENNA_USAGE_NOT_SUPPORTED:
      break;

    // Close connection
    default:

      // Close the connection
      (void)ble_peer_manager_central_close_connection(conn_handle);
      break;
  }
}

// -----------------------------------------------------------------------------
// Event / callback definitions

/**************************************************************************//**
 * Bluetooth stack event handler
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  uint8_t instance_num;
  const char *device_name = REFLECTOR_DEVICE_NAME;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
    {
      // Set TX power
      int16_t min_tx_power_x10 = SYSTEM_MIN_TX_POWER_DBM * 10;
      int16_t max_tx_power_x10 = SYSTEM_MAX_TX_POWER_DBM * 10;
      sc = sl_bt_system_set_tx_power(min_tx_power_x10,
                                     max_tx_power_x10,
                                     &min_tx_power_x10,
                                     &max_tx_power_x10);
      app_assert_status(sc);

      // Reset to initial state
      ble_peer_manager_central_init();
      ble_peer_manager_filter_init();
      cs_initiator_init();

      sc = cs_antenna_configure(CS_INITIATOR_ANTENNA_OFFSET);
      app_assert_status(sc);

      // Filter for advertised name (CS_RFLCT)
      sc = ble_peer_manager_set_filter_device_name(device_name,
                                                   strlen(device_name),
                                                   false);
      app_assert_status(sc);

      uint16_t ras_service_uuid = CS_RAS_SERVICE_UUID;
      sc = ble_peer_manager_set_filter_service_uuid16(
        (sl_bt_uuid_16_t *)&ras_service_uuid);
      app_assert_status(sc);

#ifndef SL_CATALOG_CS_INITIATOR_CLI_PRESENT
      sc = ble_peer_manager_central_create_connection();
      app_assert_status(sc);
      // Start scanning for reflector connections
#else
      log_info("CS CLI is active." NL);
#endif // SL_CATALOG_CS_INITIATOR_CLI_PRESENT

      break;
    }

    case sl_bt_evt_connection_parameters_id:
      sc = get_instance_number(evt->data.evt_connection_parameters.connection,
                               &instance_num);
      // Initiator instance not created yet
      if (sc != SL_STATUS_OK) {
        if (evt->data.evt_connection_parameters.security_mode
            != sl_bt_connection_mode1_level1) {
          sc = sl_bt_cs_read_remote_supported_capabilities(
            evt->data.evt_connection_parameters.connection);
          app_assert_status(sc);
        } else {
          sc = sl_bt_sm_increase_security(
            evt->data.evt_connection_parameters.connection);
          app_assert_status(sc);
        }
      }
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      break;

    case sl_bt_evt_sm_bonding_failed_id:
      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:
      break;
    case sl_bt_evt_gatt_mtu_exchanged_id:
    {
      initiator_config.mtu = evt->data.evt_gatt_mtu_exchanged.mtu;
    }
    break;

    case sl_bt_evt_cs_read_remote_supported_capabilities_complete_id:
    {
      uint16_t proc_interval;
      uint16_t conn_interval;
      uint8_t cs_tone_antenna_config_index_temp =
        initiator_config.cs_tone_antenna_config_idx;
      uint8_t connection =
        evt->data.evt_cs_read_remote_supported_capabilities_complete.connection;
      sc = sl_bt_cs_read_local_supported_capabilities(NULL,
                                                      NULL,
                                                      &initiator_config.num_antennas,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL);
      app_assert_status(sc);
      if (initiator_config.max_procedure_count == 0) {
        sc = cs_initiator_get_intervals(initiator_config.cs_main_mode,
                                        initiator_config.cs_sub_mode,
                                        initiator_config.procedure_scheduling,
                                        initiator_config.channel_map_preset,
                                        rtl_config.algo_mode,
                                        initiator_config.cs_tone_antenna_config_idx,
                                        initiator_config.use_real_time_ras_mode,
                                        &conn_interval,
                                        &proc_interval);
        if (sc == SL_STATUS_OK) {
          initiator_config.max_connection_interval =
            initiator_config.min_connection_interval = conn_interval;
          initiator_config.max_procedure_interval =
            initiator_config.min_procedure_interval = proc_interval;
        }
        // put remote antenna num into cs_tone_antenna_config_idx
        initiator_config.cs_tone_antenna_config_idx =
          evt->data.evt_cs_read_remote_supported_capabilities_complete.
          num_antennas;
      }
      sc = create_new_initiator_instance(connection);
      if (sc != SL_STATUS_OK) {
        (void)ble_peer_manager_central_close_connection(connection);
      }
      // set cs_tone_antenna_config_idx to default
      initiator_config.cs_tone_antenna_config_idx =
        cs_tone_antenna_config_index_temp;
      sc = get_instance_number(connection, &instance_num);
      app_assert_status(sc);

      cs_initiator_instances[instance_num].read_remote_capabilities = true;
      // Scan for new reflector connections if we have room for more
      if (num_reflector_connections < CS_INITIATOR_MAX_CONNECTIONS) {
        sc = ble_peer_manager_central_create_connection();
        app_assert_status(sc);
      }
      break;
    }
    // -------------------------------
    // This event indicates an external_signal.
    case sl_bt_evt_system_external_signal_id:
      break;

    // -------------------------------
    // Default event
    default:
      break;
  }
}

/******************************************************************************
 * BLE peer manager event handler
 *
 * @param[in] evt Event coming from the peer manager.
 *****************************************************************************/
void ble_peer_manager_on_event_initiator(ble_peer_manager_evt_type_t *event)
{
  sl_status_t sc;

  switch (event->evt_id) {
    case BLE_PEER_MANAGER_ON_CONN_OPENED_CENTRAL:
      break;
    case BLE_PEER_MANAGER_ON_CONN_CLOSED:
      sc = cs_initiator_delete(event->connection_id);
      app_assert_status(sc);

      delete_initiator_instance(event->connection_id);
      // Restart scanning for new reflector connections
      (void)ble_peer_manager_central_create_connection();
      break;

    case BLE_PEER_MANAGER_ERROR:
      break;

    default:
      break;
  }
}
