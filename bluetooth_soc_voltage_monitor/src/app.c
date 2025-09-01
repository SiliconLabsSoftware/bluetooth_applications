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
#include "sl_bt_api.h"
#include "sl_main_init.h"
#include "app_assert.h"
#include "app.h"
#include "gatt_db.h"
#include "le_voltage_monitor.h"

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
static uint8_t connection_handle;
static uint8_t volt_buf[2] = { 0 };

// Application Init.
void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
  le_voltage_monitor_init();
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
      connection_handle = evt->data.evt_connection_opened.connection;
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      le_voltage_monitor_stop();

      // Restart advertising after client has disconnected.
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

    case sl_bt_evt_gatt_server_characteristic_status_id:
      // Check if Average Voltage Characteristic changed
      if (evt->data.evt_gatt_server_characteristic_status.characteristic
          == gattdb_avg_voltage_data) {
        // client characteristic configuration changed by remote GATT client
        if (sl_bt_gatt_server_client_config
            == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.
            evt_gatt_server_characteristic_status.status_flags) {
          // Check if EFR Connect App enabled notifications
          if (sl_bt_gatt_server_disable
              != evt->data.evt_gatt_server_characteristic_status.
              client_config_flags) {
            // Start sampling data
            le_voltage_monitor_start_next();
          }
          // indication and notifications disabled
          else {
            le_voltage_monitor_stop();
          }
        }
      }
      break;

    case sl_bt_evt_system_external_signal_id:

      // External signal triggered from LDMA interrupt
      if (evt->data.evt_system_external_signal.extsignals & LE_MONITOR_SIGNAL) {
        // Get the average
        uint16_t data_mV = le_voltage_monitor_get_average_mv();

        // Multiply data_mV by 4 because IADC positive channel configuration to measure iadcPosInputAvdd (Avdd / 4)
        data_mV *= 4;

        // Convert endianess
        volt_buf[0] = (data_mV >> 8) & 0x00FF;
        volt_buf[1] = data_mV & 0x00FF;

        // Notify connected user
        sc = sl_bt_gatt_server_send_notification(connection_handle,
                                                 gattdb_avg_voltage_data,
                                                 sizeof(volt_buf),
                                                 (uint8_t *)&volt_buf);

        // Start the next measurements
        le_voltage_monitor_start_next();
      }

      if (evt->data.evt_system_external_signal.extsignals
          & LE_PRS_FEATURE_CONFIG_SIGNAL) {
        le_voltage_monitor_conifg_prs_feature();
      }

      break;

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}
