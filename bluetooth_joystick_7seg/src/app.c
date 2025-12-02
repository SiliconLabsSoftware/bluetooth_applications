/***************************************************************************//**
 * @file app.c
 * @brief Top level application functions
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
#include <stdbool.h>
#include "sl_common.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "sl_simple_button_instances.h"
#include "sl_i2cspm_instances.h"
#include "sl_spidrv_instances.h"
#include "sl_pwm_instances.h"
#include "app_assert.h"
#include "app_log.h"
#include "app.h"
#include "sparkfun_qwiic_joystick.h"
#include "mikroe_max6969.h"

#define BTN0_IRQ_EVENT                      0x01
#define SOFT_TIMER_JOYSTICK_READ            0x00

#define NOTIFICATION_JOYSTICK_DATA          (1 << 0)
#define NOTIFICATION_JOYSTICK_VALUE_X       (1 << 1)
#define NOTIFICATION_JOYSTICK_VALUE_Y       (1 << 2)
#define NOTIFICATION_JOYSTICK_BUTTON_STATUS (1 << 3)

static void notifications_enable(uint16_t characteristic,
                                 uint16_t client_config_flags);
static sl_status_t joystick_get_data(uint8_t *data_pos_x,
                                     uint8_t *data_pos_y,
                                     uint8_t *data_button_status);
static void joystick_update_data(void);

// Connection Handle
static uint8_t connection_handle = 0xff;
static uint8_t notifications_enabled = 0;
// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

static bool is_display_on = true;

static uint8_t joystick_pre_pos = 126; // ~126 is the reading when the joystick
                                       //   is in the mid-point
static uint8_t joystick_data;

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void app_init(void)
{
  sl_status_t sc;

  sc = mikroe_max6969_init(sl_spidrv_mikroe_handle, &sl_pwm_mikroe);
  if (sc != SL_STATUS_OK) {
    app_log("Warning! Failed to init led 7seg\n");
  } else {
    app_log("Led 7seg initialized\n");
  }

  sc = sparkfun_joystick_init(sl_i2cspm_qwiic, SPARKFUN_JOYSTICK_DEFAULT_ADDR);
  if (sc != SL_STATUS_OK) {
    app_log("Warning! Failed to init Joystick\n");
  } else {
    app_log("Joystick initialized\n");
  }

  mikroe_max6969_display_number(0, MIKROE_UTM7SEGR_NO_DOT);
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
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
  size_t len;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert(sc == SL_STATUS_OK,
                 "[E: 0x%04x] Failed to create advertising set\n",
                 (int)sc);

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(
        advertising_set_handle,
        sl_bt_advertiser_general_discoverable);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      app_assert(sc == SL_STATUS_OK,
                 "[E: 0x%04x] Failed to set advertising timing\n",
                 (int)sc);
      // Start general advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(
        advertising_set_handle,
        sl_bt_legacy_advertiser_connectable);
      app_assert(sc == SL_STATUS_OK,
                 "[E: 0x%04x] Failed to start advertising\n",
                 (int)sc);
      app_log("boot event - starting advertising\r\n");

      /* Set a timer to call joystick_update_data() every 100 ms */
      sc =
        sl_bt_system_set_lazy_soft_timer(3276, SOFT_TIMER_JOYSTICK_READ, 0, 0);
      app_assert(sc == SL_STATUS_OK,
                 "[E: 0x%04x] Failed to start a software timer\n",
                 (int)sc);
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      app_log("Connection opened\r\n");
      // Save connection handle for future reference
      connection_handle = evt->data.evt_connection_opened.connection;
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      app_log("connection closed, reason: 0x%2.2x\r\n",
              evt->data.evt_connection_closed.reason);
      connection_handle = 0xff;
      notifications_enabled = 0;
      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(
        advertising_set_handle,
        sl_bt_legacy_advertiser_connectable);
      app_assert(sc == SL_STATUS_OK,
                 "[E: 0x%04x] Failed to start advertising\n",
                 (int)sc);
      break;

    // -------------------------------
    // This event indicates that the value of an attribute in the local GATT
    //   database
    // was changed by a remote GATT client.
    case sl_bt_evt_gatt_server_attribute_value_id:
      if (evt->data.evt_gatt_server_attribute_value.attribute
          == gattdb_joystick_data) {
        sc = sl_bt_gatt_server_read_attribute_value(gattdb_joystick_data,
                                                    0,
                                                    1,
                                                    &len,
                                                    &joystick_data);
        app_assert(sc == SL_STATUS_OK,
                   "[E: 0x%04x] Failed to read attribute\n",
                   (int)sc);

        mikroe_max6969_display_number(joystick_data, MIKROE_UTM7SEGR_NO_DOT);
        // if the display is off then turn on it
        if (is_display_on == false) {
          is_display_on = true;
          mikroe_max6969_set_contrast(50);
        }
      }
      break;

    /* When the remote device subscribes for notification,
     * start a timer to send out notifications periodically */
    case sl_bt_evt_gatt_server_characteristic_status_id:
      if (evt->data.evt_gatt_server_characteristic_status.status_flags
          != sl_bt_gatt_server_client_config) {
        break;
      }
      notifications_enable(
        evt->data.evt_gatt_server_characteristic_status.characteristic,
        evt->data.evt_gatt_server_characteristic_status.client_config_flags
        );
      break;

    // -------------------------------
    // This event indicates that a soft timer has lapsed.
    case sl_bt_evt_system_soft_timer_id:
      if (evt->data.evt_system_soft_timer.handle == SOFT_TIMER_JOYSTICK_READ) {
        joystick_update_data();
      }
      break;

    // -------------------------------
    // External signal indication (comes from the interrupt handler)
    case sl_bt_evt_system_external_signal_id:
      if (evt->data.evt_system_external_signal.extsignals
          == BTN0_IRQ_EVENT) {
        if (is_display_on == true) {
          is_display_on = false;
          mikroe_max6969_set_contrast(0);
        } else {
          is_display_on = true;
          mikroe_max6969_set_contrast(50);
        }
      }
      break;
    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

/**************************************************************************//**
 * Simple Button
 * Button state changed callback
 * @param[in] handle Button event handle
 *****************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  // Button pressed.
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&sl_button_btn0 == handle) {
      sl_bt_external_signal(BTN0_IRQ_EVENT);
    }
  }
}

static void notifications_enable(uint16_t characteristic,
                                 uint16_t client_config_flags)
{
  switch (characteristic) {
    case gattdb_joystick_data:
      if (client_config_flags) {
        app_log("Enable notification: Joystick data\n");
        notifications_enabled |= NOTIFICATION_JOYSTICK_DATA;
      } else {
        notifications_enabled &= ~NOTIFICATION_JOYSTICK_DATA;
      }
      break;

    case gattdb_joystick_value_x:
      if (client_config_flags) {
        app_log("Enable notification: Joystick X\n");
        notifications_enabled |= NOTIFICATION_JOYSTICK_VALUE_X;
      } else {
        notifications_enabled &= ~NOTIFICATION_JOYSTICK_VALUE_X;
      }
      break;

    case gattdb_joystick_value_y:
      if (client_config_flags) {
        app_log("Enable notification: Joystick Y\n");
        notifications_enabled |= NOTIFICATION_JOYSTICK_VALUE_Y;
      } else {
        notifications_enabled &= ~NOTIFICATION_JOYSTICK_VALUE_Y;
      }
      break;

    case gattdb_joystick_button_status:
      if (client_config_flags) {
        app_log("Enable notification: Joystick Button Status\n");
        notifications_enabled |= NOTIFICATION_JOYSTICK_BUTTON_STATUS;
      } else {
        notifications_enabled &= ~NOTIFICATION_JOYSTICK_BUTTON_STATUS;
      }
      break;
  }
}

/**************************************************************************//**
 * For simplicity, reading is the current vertical position from joystick.
 * And only look at the MSB and get an 8-bit reading (for 256 positions).
 *****************************************************************************/
static sl_status_t joystick_get_data(uint8_t *data_pos_x,
                                     uint8_t *data_pos_y,
                                     uint8_t *data_button_status)
{
  sl_status_t sc;

  uint16_t data_pos_tmp;
  uint8_t data_button_status_tmp;

  sc = sparkfun_joystick_read_horizontal_position(&data_pos_tmp);
  if (SL_STATUS_OK != sc) {
    return sc;
  }

  /* In the registers for the joystick position, the MSB contains the first
   *    8 bits of the 10-bit ADC value and the LSB contains the last two bits.
   *    As an example, this is how the library converts the two registers back
   *   to
   *    a 10-bit value
   *    For simplicity, reading is the current vertical position from joystick.
   *    And only look at the MSB and get an 8-bit reading (for 256 positions).*/
  *data_pos_x = (uint8_t)(data_pos_tmp >> 2);

  sc = sparkfun_joystick_read_vertical_position(&data_pos_tmp);
  if (SL_STATUS_OK != sc) {
    return sc;
  }
  *data_pos_y = (uint8_t)(data_pos_tmp >> 2);

  // Read current Button Status
  sc = sparkfun_joystick_read_button_position(&data_button_status_tmp);
  if (SL_STATUS_OK != sc) {
    return sc;
  }
  *data_button_status = data_button_status_tmp;

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * Update the current postion from joystick board
 *****************************************************************************/
static void joystick_update_data(void)
{
  sl_status_t sc;
  size_t len;
  uint8_t joystick_last_pos_x = 0, joystick_last_pos_y = 0;
  uint8_t joystick_last_button_status = 0;
  uint8_t joystick_current_pos_x = 0, joystick_current_pos_y = 0;
  uint8_t joystick_current_button_status = 0;

//  app_log("update data\n");
  // Read the current position from joystick board
  sc = joystick_get_data(&joystick_current_pos_x,
                         &joystick_current_pos_y,
                         &joystick_current_button_status);
  if (sc != SL_STATUS_OK) {
    app_log("Warning! Invalid Joystick reading\n");
  }

  if (abs(joystick_pre_pos - joystick_current_pos_y) > 4) {
    if (joystick_current_pos_y == 255) {
      joystick_data++;
    } else if ((joystick_current_pos_y == 0) && (joystick_data > 0)) {
      joystick_data--;
    }

    // update the value to 7seg Click
    mikroe_max6969_display_number(joystick_data, MIKROE_UTM7SEGR_NO_DOT);
    app_log("position: %d\n", joystick_data);
    // if the display is off then turn on it
    if (is_display_on == false) {
      is_display_on = true;
      mikroe_max6969_set_contrast(50);
    }
    // Write the value of an attribute in the local GATT database
    sc = sl_bt_gatt_server_write_attribute_value(gattdb_joystick_data,
                                                 0,
                                                 1,
                                                 &joystick_data);
    app_assert(sc == SL_STATUS_OK,
               "[E: 0x%04x] Failed to write the value of an attribute\n",
               (int)sc);
  }

  joystick_pre_pos = joystick_current_pos_y;

  if (notifications_enabled & NOTIFICATION_JOYSTICK_DATA) {
    sc = sl_bt_gatt_server_send_notification(connection_handle,
                                             gattdb_joystick_data,
                                             1,
                                             &joystick_data);
    app_assert(sc == SL_STATUS_OK,
               "[E: 0x%04x] Failed to send notifications\n",
               (int)sc);
  }

  // Read last X
  sc = sl_bt_gatt_server_read_attribute_value(gattdb_joystick_value_x,
                                              0,
                                              1,
                                              &len,
                                              &joystick_last_pos_x);
  app_assert(sc == SL_STATUS_OK,
             "[E: 0x%04x] Failed to read attribute: X position value\n",
             (int)sc);
  // Write the value of an attribute in the local GATT database
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_joystick_value_x,
                                               0,
                                               1,
                                               &joystick_current_pos_x);
  app_assert(
    sc == SL_STATUS_OK,
    "[E: 0x%04x] Failed to write attribute: X position value\n",
    (int)sc);
  if (joystick_last_pos_x != joystick_current_pos_x) {
    app_log("Current joystick X: %d\n",
            (int)joystick_current_pos_x);
    if (notifications_enabled & NOTIFICATION_JOYSTICK_VALUE_X) {
      sc = sl_bt_gatt_server_send_notification(connection_handle,
                                               gattdb_joystick_value_x,
                                               1,
                                               &joystick_current_pos_x);
      app_assert(
        sc == SL_STATUS_OK,
        "[E: 0x%04x] Failed to send notifications: X position value\n",
        (int)sc);
    }
  }

  // Read last Y
  sc = sl_bt_gatt_server_read_attribute_value(gattdb_joystick_value_y,
                                              0,
                                              1,
                                              &len,
                                              &joystick_last_pos_y);
  app_assert(sc == SL_STATUS_OK,
             "[E: 0x%04x] Failed to read attribute: Y position value\n",
             (int)sc);
  // Write the value of an attribute in the local GATT database
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_joystick_value_y,
                                               0,
                                               1,
                                               &joystick_current_pos_y);
  app_assert(
    sc == SL_STATUS_OK,
    "[E: 0x%04x] Failed to write attribute: Y position value\n",
    (int)sc);
  if (joystick_last_pos_y != joystick_current_pos_y) {
    app_log("Current joystick Y: %d\n",
            (int)joystick_current_pos_y);
    if (notifications_enabled & NOTIFICATION_JOYSTICK_VALUE_Y) {
      sc = sl_bt_gatt_server_send_notification(connection_handle,
                                               gattdb_joystick_value_y,
                                               1,
                                               &joystick_current_pos_y);
      app_assert(
        sc == SL_STATUS_OK,
        "[E: 0x%04x] Failed to send notifications: Y position value\n",
        (int)sc);
    }
  }

  // Read last Button Status
  sc = sl_bt_gatt_server_read_attribute_value(gattdb_joystick_button_status,
                                              0,
                                              1,
                                              &len,
                                              &joystick_last_button_status);
  app_assert(sc == SL_STATUS_OK,
             "[E: 0x%04x] Failed to read attribute: button status\n",
             (int)sc);
  // Write the value of an attribute in the local GATT database
  sc = sl_bt_gatt_server_write_attribute_value(
    gattdb_joystick_button_status,
    0,
    1,
    &joystick_current_button_status);
  app_assert(
    sc == SL_STATUS_OK,
    "[E: 0x%04x] Failed to write attribute: button status\n",
    (int)sc);
  if (joystick_last_button_status != joystick_current_button_status) {
    app_log("Current joystick button status: %d\n",
            (int)joystick_current_button_status);
    if (notifications_enabled & NOTIFICATION_JOYSTICK_BUTTON_STATUS) {
      sc = sl_bt_gatt_server_send_notification(
        connection_handle,
        gattdb_joystick_button_status,
        1,
        &joystick_current_button_status);
      app_assert(sc == SL_STATUS_OK,
                 "[E: 0x%04x] Failed to send notifications: button status\n",
                 (int)sc);
    }
  }
}
