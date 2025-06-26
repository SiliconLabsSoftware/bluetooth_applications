/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 *    1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "app.h"
#include "app_log.h"
#include "app_assert.h"
#include "gatt_db.h"
#include "sl_sleeptimer.h"
#include "sl_joystick.h"

#define JOYSTICK_EXT_SIG    (0x01)
#define JOYSTICK_NOTIF_FLAG (0x02)
#define JOYSTICK_DEBUG      (0x01)

static uint8_t advertising_set_handle = 0xff;
static uint8_t notification_enabled = 0;
static sl_joystick_t sl_joystick_handle = JOYSTICK_HANDLE_DEFAULT;
static sl_sleeptimer_timer_handle_t joystick_timer;

typedef struct {
  int8_t x_axis;   // X-axis (-127 to 127)
  int8_t y_axis;   // Y-axis (-127 to 127)
  uint8_t button : 1;  // Button state (0 or 1)
  uint8_t padding : 7; // Padding to align to byte boundary
} joystick_report_t;

joystick_report_t joystick_report = {
  .x_axis = 0,      // Centered X
  .y_axis = 0,      // Centered Y
  .button = 0,    // No button pressed
  .padding = 0
};

/* Report Formats:
 *
 * Joystick
 *
 * Byte Size Description Range
 * 0 1 byte(int8_t) X-axis position -127 to 127
 * 1 1 byte(int8_t) Y-axis position -127 to 127
 * 2 1 byte(uint8_t) Button state (bitfield) 0x00 to 0xFF
 *
 */
static int8_t input_report_data[3] = { 0, 0, 0 };

void joystick_timer_cb(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;
  sl_joystick_position_t pos;
  bool button_pressed = false;
  int8_t x = 0;
  int8_t y = 0;
  if (sl_joystick_get_position(&sl_joystick_handle, &pos) != SL_STATUS_OK) {
    // Not ready or error, zero out axes
    joystick_report.x_axis = 0;
    joystick_report.y_axis = 0;
    joystick_report.button = button_pressed ? true : false;
    joystick_report.padding = 0;
    return;
  }

  switch (pos) {
    case JOYSTICK_N:  y = -127; break;
    case JOYSTICK_S:  y = 127; break;
    case JOYSTICK_E:  x = 127; break;
    case JOYSTICK_W:  x = -127; break;
#if ENABLE_SECONDARY_DIRECTIONS == 1
    case JOYSTICK_NE: x = 127;  y = -127;  break;
    case JOYSTICK_NW: x = -127; y = -127;  break;
    case JOYSTICK_SE: x = 127;  y = 127; break;
    case JOYSTICK_SW: x = -127; y = 127; break;
    case JOYSTICK_C:  button_pressed ^= 1; break;
    case JOYSTICK_NONE:
#endif
    default:
      x = 0;
      y = 0;
      break;
  }

  joystick_report.x_axis = x;
  joystick_report.y_axis = y;
  joystick_report.button = button_pressed ? 1 : 0;
  joystick_report.padding = 0;

  sl_bt_external_signal(JOYSTICK_EXT_SIG);
}

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  sl_joystick_init(&sl_joystick_handle);
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                             //
  // This is called infinitely.                                             //
  // Do not call blocking functions from here!                              //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header)) {
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

      app_log("boot event - starting advertising\r\n");

      sc = sl_bt_sm_configure(0, sl_bt_sm_io_capability_noinputnooutput);
      app_assert_status(sc);
      sc = sl_bt_sm_set_bondable_mode(1);
      app_assert_status(sc);

      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

    case sl_bt_evt_connection_opened_id:
      app_log("connection opened\r\n");
      sc =
        sl_bt_sm_increase_security(evt->data.evt_connection_opened.connection);
      app_assert_status(sc);
      break;

    case sl_bt_evt_connection_closed_id:

      sl_sleeptimer_stop_timer(&joystick_timer);

      sl_joystick_stop(&sl_joystick_handle);

      app_log("connection closed, reason: 0x%2.2x\r\n",
              evt->data.evt_connection_closed.reason);
      notification_enabled = 0;
      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

    case sl_bt_evt_sm_bonded_id:
      app_log("successful bonding\r\n");
      break;

    case sl_bt_evt_sm_bonding_failed_id:
      app_log("bonding failed, reason 0x%2X\r\n",
              evt->data.evt_sm_bonding_failed.reason);

      /* Previous bond is broken, delete it and close connection,
       *  host must retry at least once */
      sc = sl_bt_sm_delete_bondings();
      if (sc != SL_STATUS_OK) {
        app_log("Problem deleting bondings");
      }
      sc = sl_bt_connection_close(evt->data.evt_sm_bonding_failed.connection);
      if (sc != SL_STATUS_OK) {
        app_log("Bond broken, delete it and close connection. Retry once!!!");
      }
      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:

      if (evt->data.evt_gatt_server_characteristic_status.characteristic
          == gattdb_report_joystick) {
        // client characteristic configuration changed by remote GATT client
        if ((evt->data.evt_gatt_server_characteristic_status.status_flags
             == sl_bt_gatt_server_client_config)
            && (evt->data.evt_gatt_server_characteristic_status.
                client_config_flags
                == sl_bt_gatt_notification)) {
          app_log("Notification status changed: 0x%02x\r\n",
                  evt->data.evt_gatt_server_characteristic_status.client_config_flags);
          notification_enabled |= JOYSTICK_NOTIF_FLAG;
          sl_joystick_start(&sl_joystick_handle);
          sl_sleeptimer_start_periodic_timer_ms(&joystick_timer,
                                                50,
                                                joystick_timer_cb,
                                                NULL,
                                                32,
                                                0);
        } else {
          notification_enabled &= ~JOYSTICK_NOTIF_FLAG;
          sl_sleeptimer_stop_timer(&joystick_timer);
          sl_joystick_stop(&sl_joystick_handle);
        }
      }
      break;

    case sl_bt_evt_system_external_signal_id:
      if (notification_enabled == 0) {
        break;
      }
      memset(input_report_data, 0, sizeof(input_report_data));
      if (evt->data.evt_system_external_signal.extsignals & JOYSTICK_EXT_SIG) {
        input_report_data[0] = joystick_report.x_axis;
        input_report_data[1] = joystick_report.y_axis;
        input_report_data[2] =
          (uint8_t)((joystick_report.button & 0x01)
                    | (joystick_report.padding << 1));

#if JOYSTICK_DEBUG == 1
        app_log("xValue: %d, yValue: %d, button: %d\r\n",
                input_report_data[0],
                input_report_data[1],
                input_report_data[2]);
#endif
        joystick_report.x_axis = 0;
        joystick_report.y_axis = 0;
        sc = sl_bt_gatt_server_notify_all(gattdb_report_joystick,
                                          sizeof(input_report_data),
                                          (uint8_t *)input_report_data);
        if (sc != SL_STATUS_OK) {
          app_log("Notify failed! Error code: 0x%04x\r\n", (uint8_t)sc);
        }

#if JOYSTICK_DEBUG == 1
        app_log("Joystick report was sent\r\n");
#endif
      }
      break;

    default:
      break;
  }
}
