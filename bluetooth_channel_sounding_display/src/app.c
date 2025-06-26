/***************************************************************************//**
 * @file app.c
 * @brief CS Initiator example application logic
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
#include "sl_bluetooth.h"
#include "sl_component_catalog.h"
#include "sl_common.h"
#include "app_assert.h"
#include "gatt_db.h"

// app content
#include "app.h"
#include "app_config.h"
#include "trace.h"

// initiator content
#include "cs_antenna.h"
#include "cs_initiator.h"
#include "cs_initiator_client.h"
#include "cs_initiator_config.h"
#include "cs_initiator_display.h"

// other required content
#include "ble_peer_manager_common.h"
#include "ble_peer_manager_connections.h"
#include "ble_peer_manager_central.h"
#include "ble_peer_manager_filter.h"

#ifdef SL_CATALOG_CS_INITIATOR_CLI_PRESENT
#include "cs_initiator_cli.h"
#endif // SL_CATALOG_CS_INITIATOR_CLI_PRESENT

#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

// -----------------------------------------------------------------------------
// Macros

#define NL                               APP_LOG_NL
#define APP_PREFIX                       "[APP] "
#define INSTANCE_PREFIX                  "[%u] "
#define APP_INSTANCE_PREFIX              APP_PREFIX INSTANCE_PREFIX
#define SIGNAL_NOTIFY                    1

// -----------------------------------------------------------------------------
// Static function declarations

static uint8_t get_algo_mode(void);
static char *antenna_usage_to_str(const cs_initiator_config_t *config);
static void cs_on_result(const cs_result_t *result,
                         const sl_rtl_cs_procedure *cs_procedure,
                         const void *user_data);
static void cs_on_intermediate_result(
  const cs_intermediate_result_t *intermediate_result,
  const void *user_data);
static void cs_on_error(uint8_t conn_handle,
                        cs_error_event_t err_evt,
                        sl_status_t sc);

// -----------------------------------------------------------------------------
// Static variables

static bool measurement_arrived = false;
static bool measurement_progress_changed = false;
static bool antenna_set_pbr = false;
static bool antenna_set_rtt = false;
static uint32_t measurement_cnt = 0u;
static cs_initiator_config_t initiator_config = INITIATOR_CONFIG_DEFAULT;
static rtl_config_t rtl_config = RTL_CONFIG_DEFAULT;

static cs_result_t measurement;
static cs_intermediate_result_t measurement_progress;

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

/******************************************************************************
 * Application Init
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  sl_status_t sc = SL_STATUS_OK;

  trace_init();

  // initialize measurement variable
  memset(&measurement, 0u, sizeof(measurement));
  memset(&measurement_progress, 0u, sizeof(measurement_progress));
  measurement.connection = SL_BT_INVALID_CONNECTION_HANDLE;

  // Set configuration parameters
  rtl_config.algo_mode = get_algo_mode();
  cs_initiator_apply_channel_map_preset(rtl_config.channel_map_preset,
                                        rtl_config.cs_parameters.channel_map);

  // Log configuration parameters
  log_info("+-[CS User Friendly Display by Silicon Labs]-+" NL);
  log_info("+--------------------------------------------+" NL);
  log_info(APP_PREFIX "Antenna offset: wire%s" NL,
           CS_INITIATOR_ANTENNA_OFFSET ? "d" : "less");
  log_info(APP_PREFIX "Minimum CS procedure interval: %u" NL,
           initiator_config.min_procedure_interval);
  log_info(APP_PREFIX "Maximum CS procedure interval: %u" NL,
           initiator_config.max_procedure_interval);
  log_info(APP_PREFIX "CS mode: %s (%u)" NL,
           (initiator_config.cs_mode == sl_bt_cs_mode_pbr) ? "PBR" : "RTT",
           initiator_config.cs_mode);
  log_info(APP_PREFIX "Requested antenna usage: %s" NL,
           antenna_usage_to_str(&initiator_config));
  log_info(APP_PREFIX "Object tracking mode: %s" NL,
           rtl_config.algo_mode == SL_RTL_CS_ALGO_MODE_STATIC_HIGH_ACCURACY
           ? "stationary object tracking"
           : "moving object tracking (up to 5 km/h)");
  log_info(APP_PREFIX "CS channel map preset: %d" NL,
           rtl_config.channel_map_preset);
  log_info(
    APP_PREFIX "CS channel map: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X" NL,
    rtl_config.cs_parameters.channel_map[0],
    rtl_config.cs_parameters.channel_map[1],
    rtl_config.cs_parameters.channel_map[2],
    rtl_config.cs_parameters.channel_map[3],
    rtl_config.cs_parameters.channel_map[4],
    rtl_config.cs_parameters.channel_map[5],
    rtl_config.cs_parameters.channel_map[6],
    rtl_config.cs_parameters.channel_map[7],
    rtl_config.cs_parameters.channel_map[8],
    rtl_config.cs_parameters.channel_map[9]);
  log_info(APP_PREFIX "RSSI reference TX power @ 1m: %d dBm" NL,
           (int)initiator_config.rssi_ref_tx_power);
  log_info("+-------------------------------------------------------+" NL);

  sc = cs_initiator_display_init();
  app_assert_status_f(sc, "cs_display_init failed");

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

/******************************************************************************
 * Application Process Action
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  trace_step();
  if (measurement_arrived) {
    measurement_arrived = false;
    // send notification to client
    sl_bt_external_signal(SIGNAL_NOTIFY);

    // write results to the display & to the iostream
    log_info(APP_INSTANCE_PREFIX "# %04lu ---" NL,
             measurement.connection,
             measurement_cnt);

    log_info(APP_INSTANCE_PREFIX "Measurement result: %lu mm" NL,
             measurement.connection,
             (uint32_t)(measurement.distance * 1000.f));

    cs_initiator_display_distance_measurement(measurement.distance);
    cs_initiator_display_progress_bar(measurement.distance);

    uint32_t likeliness_whole = (uint32_t)measurement.likeliness;
    uint32_t likeliness_frac =
      (uint32_t)((measurement.likeliness - (float)likeliness_whole) * 100.0f);
    log_info(APP_INSTANCE_PREFIX "Measurement likeliness: %lu.%02lu" NL,
             measurement.connection,
             likeliness_whole,
             likeliness_frac);

    log_info(APP_INSTANCE_PREFIX "RSSI distance: %lu mm" NL,
             measurement.connection,
             (uint32_t)(measurement.rssi_distance * 1000.f));

    if (!isnan(measurement.bit_error_rate)) {
      uint8_t ber_whole = (uint8_t)measurement.bit_error_rate;
      uint32_t ber_frac =
        (uint32_t)((measurement.bit_error_rate - (float)ber_whole) * 1e6);
      log_info(APP_INSTANCE_PREFIX "CS bit error rate: %u.%06lu" NL,
               measurement.connection,
               ber_whole,
               ber_frac);
    }
  } else if (measurement_progress_changed) {
    // write measurement progress to the display without changing the last valid
    // measurement results
    measurement_progress_changed = false;
    log_info(APP_INSTANCE_PREFIX "# %04lu ---" NL,
             measurement_progress.connection,
             measurement_cnt);

    uint32_t percent_whole = (uint32_t)measurement_progress.progress_percentage;
    uint32_t percent_frac =
      (uint32_t)((measurement_progress.progress_percentage
                  - (float)percent_whole) * 100.0f);
    log_info(APP_INSTANCE_PREFIX "Estimation in progress: %lu.%02lu %%" NL,
             measurement_progress.connection,
             percent_whole,
             percent_frac);
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
#if CS_INITIATOR_DEFAULT_ALGO_MODE == SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC
#define CS_INITIATOR_ALTERNATIVE_ALGO_MODE \
  SL_RTL_CS_ALGO_MODE_STATIC_HIGH_ACCURACY
#else
#define CS_INITIATOR_ALTERNATIVE_ALGO_MODE SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC
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
 * Get requested antenna usage configuration as string
 *****************************************************************************/
static char *antenna_usage_to_str(const cs_initiator_config_t *config)
{
  if (config->cs_mode == sl_bt_cs_mode_rtt) {
    switch (config->cs_sync_antenna_req) {
      case CS_SYNC_ANTENNA_1:
        return "antenna ID 1";
      case CS_SYNC_ANTENNA_2:
        return "antenna ID 2";
      case CS_SYNC_SWITCHING:
        return "switch between all antenna IDs";
      default:
        return "unknown";
    }
  } else {
    switch (config->cs_tone_antenna_config_idx_req) {
      case CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY:
        return "single antenna on both sides (1:1)";
      case CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R:
        return "dual antenna initiator & single antenna reflector (2:1)";
      case CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R:
        return "single antenna initiator & dual antenna reflector (1:2)";
      case CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY:
        return "dual antennas on both sides (2:2)";
      default:
        return "unknown";
    }
  }
}

/******************************************************************************
 * Extract measurement results
 *****************************************************************************/
static void cs_on_result(const cs_result_t *result,
                         const sl_rtl_cs_procedure *cs_procedure,
                         const void *user_data)
{
  (void)cs_procedure;
  (void)user_data;
  if (result != NULL) {
    memcpy(&measurement, result, sizeof(measurement));
    measurement_arrived = true;
    measurement_cnt++;
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
  if (intermediate_result != NULL) {
    memcpy(&measurement_progress, intermediate_result,
           sizeof(measurement_progress));
    measurement_progress_changed = true;
  }
}

/******************************************************************************
 * CS error handler
 *****************************************************************************/
static void cs_on_error(uint8_t conn_handle,
                        cs_error_event_t err_evt,
                        sl_status_t sc)
{
  switch (err_evt) {
    // Assert
    case CS_ERROR_EVENT_CS_PROCEDURE_STOP_TIMER_FAILED:
    case CS_ERROR_EVENT_CS_PROCEDURE_UNEXPECTED_DATA:
      app_assert(false,
                 APP_INSTANCE_PREFIX "Unrecoverable CS procedure error happened!"
                                     "[E: 0x%x sc: 0x%lx]" NL,
                 conn_handle,
                 err_evt,
                 (unsigned long)sc);
      break;
    // Discard
    case CS_ERROR_EVENT_RTL_PROCESS_ERROR:
      log_error(APP_INSTANCE_PREFIX "RTL processing error happened!"
                                    "[E: 0x%x sc: 0x%lx]" NL,
                conn_handle,
                err_evt,
                (unsigned long)sc);
      break;

    // Antenna usage not supported
    case CS_ERROR_EVENT_INITIATOR_PBR_ANTENNA_USAGE_NOT_SUPPORTED:
      if (antenna_set_pbr) {
        log_error(APP_INSTANCE_PREFIX "The requested PBR antenna configuration is not supported!"
                                      " Will use the closest one and continue."
                                      "[E: 0x%x sc: 0x%lx]" NL,
                  conn_handle,
                  err_evt,
                  (unsigned long)sc);
      } else {
        log_info(APP_INSTANCE_PREFIX "Default PBR antenna configuration not supported!"
                                     " Will use the closest one and continue."
                                     "[E: 0x%x sc: 0x%lx]" APP_LOG_NL,
                 conn_handle,
                 err_evt,
                 (unsigned long)sc);
      }
      break;
    case CS_ERROR_EVENT_INITIATOR_RTT_ANTENNA_USAGE_NOT_SUPPORTED:
      if (antenna_set_rtt) {
        log_error(APP_INSTANCE_PREFIX "The requested RTT antenna configuration is not supported!"
                                      " Will use the closest one and continue."
                                      "[E: 0x%x sc: 0x%lx]" APP_LOG_NL,
                  conn_handle,
                  err_evt,
                  (unsigned long)sc);
      } else {
        log_info(APP_INSTANCE_PREFIX "Default RTT antenna configuration not supported!"
                                     " Will use the closest one and continue."
                                     "[E: 0x%x sc: 0x%lx]" APP_LOG_NL,
                 conn_handle,
                 err_evt,
                 (unsigned long)sc);
      }
      break;

    // Close connection
    default:
      log_error(APP_INSTANCE_PREFIX "Error happened! Closing connection."
                                    "[E: 0x%x sc: 0x%lx]" NL,
                conn_handle,
                err_evt,
                (unsigned long)sc);
      // Common errors
      if (err_evt == CS_ERROR_EVENT_TIMER_ELAPSED) {
        log_error(APP_INSTANCE_PREFIX "Operation timeout." NL, conn_handle);
      } else if (err_evt
                 == CS_ERROR_EVENT_INITIATOR_FAILED_TO_INCREASE_SECURITY) {
        log_error(APP_INSTANCE_PREFIX "Security level increase failed." NL,
                  conn_handle);
      }
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
  const char *device_name = INITIATOR_DEVICE_NAME;

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
      log_info(APP_PREFIX "Minimum system TX power is set to: %d dBm" NL,
               min_tx_power_x10 / 10);
      log_info(APP_PREFIX "Maximum system TX power is set to: %d dBm" NL,
               max_tx_power_x10 / 10);

      // Reset to initial state
      ble_peer_manager_central_init();
      ble_peer_manager_filter_init();
      cs_initiator_init();

      // Print the Bluetooth address
      bd_addr address;
      uint8_t address_type;
      sc = sl_bt_gap_get_identity_address(&address, &address_type);
      app_assert_status(sc);
      log_info(
        APP_PREFIX "Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X\n",
        address_type ? "static random" : "public device",
        address.addr[5],
        address.addr[4],
        address.addr[3],
        address.addr[2],
        address.addr[1],
        address.addr[0]);

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
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      app_assert_status(sc);
      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);
      log_info(APP_PREFIX "Advertise started..." NL);

      sc = cs_antenna_configure(CS_INITIATOR_ANTENNA_OFFSET);
      app_assert_status(sc);

      // Filter for advertised name (CS_RFLCT)
      sc = ble_peer_manager_set_filter_device_name(device_name,
                                                   strlen(device_name),
                                                   false);
      app_assert_status(sc);

#ifndef SL_CATALOG_CS_INITIATOR_CLI_PRESENT
      sc = ble_peer_manager_central_create_connection();
      app_assert_status(sc);
      // Start scanning for reflector connections
      log_info(APP_PREFIX "Scanning started for reflector connections..." NL);
#else
      log_info("CS CLI is active." NL);
#endif // SL_CATALOG_CS_INITIATOR_CLI_PRESENT
      break;
    }

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

    // -------------------------------
    // This event indicates an external_signal.
    case sl_bt_evt_system_external_signal_id:
      if (evt->data.evt_system_external_signal.extsignals & SIGNAL_NOTIFY) {
        sl_bt_gatt_server_notify_all(gattdb_distance,
                                     sizeof(measurement.distance),
                                     (uint8_t *)&measurement.distance);
      }
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
  const bd_addr *address;

  switch (event->evt_id) {
    case BLE_PEER_MANAGER_ON_CONN_OPENED_CENTRAL:
      address = ble_peer_manager_get_bt_address(event->connection_id);
      log_info(APP_INSTANCE_PREFIX "Connection opened as central with CS Reflector"
                                   " '%02X:%02X:%02X:%02X:%02X:%02X'" NL,
               event->connection_id,
               address->addr[5],
               address->addr[4],
               address->addr[3],
               address->addr[2],
               address->addr[1],
               address->addr[0]);
#ifdef SL_CATALOG_CS_INITIATOR_CLI_PRESENT
      if (cs_initiator_cli_get_antenna_config_index()
          != initiator_config.cs_tone_antenna_config_idx_req) {
        antenna_set_pbr = true;
      }
      initiator_config.cs_tone_antenna_config_idx_req =
        cs_initiator_cli_get_antenna_config_index();
      if (cs_initiator_cli_get_cs_sync_antenna_usage()
          != initiator_config.cs_sync_antenna_req) {
        antenna_set_rtt = true;
      }
      initiator_config.cs_sync_antenna_req =
        cs_initiator_cli_get_cs_sync_antenna_usage();
      initiator_config.cs_mode = cs_initiator_cli_get_mode();
      initiator_config.conn_phy = cs_initiator_cli_get_conn_phy();
      rtl_config.algo_mode = cs_initiator_cli_get_algo_mode();
      rtl_config.channel_map_preset = cs_initiator_cli_get_preset();
      cs_initiator_apply_channel_map_preset(rtl_config.channel_map_preset,
                                            rtl_config.cs_parameters.channel_map);
#endif // SL_CATALOG_CS_INITIATOR_CLI_PRESENT
      sc = cs_initiator_create(event->connection_id,
                               &initiator_config,
                               &rtl_config,
                               cs_on_result,
                               cs_on_intermediate_result,
                               cs_on_error,
                               NULL);
      if (sc != SL_STATUS_OK) {
        log_error(APP_INSTANCE_PREFIX "Failed to create initiator instance, "
                                      "error:0x%lx" NL,
                  event->connection_id,
                  sc);
        (void)ble_peer_manager_central_close_connection(event->connection_id);
      } else {
        log_info(APP_INSTANCE_PREFIX "New initiator instance created" NL,
                 event->connection_id);
      }
      measurement_cnt = 0u;
      break;

    case BLE_PEER_MANAGER_ON_CONN_CLOSED:
      log_info(APP_INSTANCE_PREFIX "Connection closed" NL,
               event->connection_id);
      sc = cs_initiator_delete(event->connection_id);
      // Start scanning for reflector connections
      sc = ble_peer_manager_central_create_connection();
      app_assert_status(sc);
      log_info(APP_PREFIX "Scanning started for reflector connections..." NL);
      break;

    case BLE_PEER_MANAGER_ERROR:
      log_error(APP_INSTANCE_PREFIX "Peer Manager error" NL,
                event->connection_id);
      break;

    default:
      log_info(APP_INSTANCE_PREFIX "Unhandled Peer Manager event (%u)" NL,
               event->connection_id,
               event->evt_id);
      break;
  }
}
