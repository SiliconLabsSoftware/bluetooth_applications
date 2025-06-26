/***************************************************************************//**
 * @file app.c
 * @brief Main application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 ********************************************************************************
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
 * This code has been minimally tested to ensure that it builds and is suitable
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/
#include "app_ph2.h"
#include "app_micro_oled.h"
#include "sl_common.h"
#include "sl_bt_api.h"
#include "gatt_db.h"
#include "app.h"
#include "sl_simple_button_instances.h"

// -------------------------------
// Advertising flags (common)
#define ADVERTISE_FLAGS_LENGTH                      (2)
#define ADVERTISE_FLAGS_TYPE                        (0x01)

/** Bit mask for flags advertising data type. */
#define ADVERTISE_FLAGS_LE_LIMITED_DISCOVERABLE     (0x01)
#define ADVERTISE_FLAGS_LE_GENERAL_DISCOVERABLE     (0x02)
#define ADVERTISE_FLAGS_BR_EDR_NOT_SUPPORTED        (0x04)

// -------------------------------
// Scan Response
#define ADVERTISE_MANDATORY_DATA_LENGTH             (5)
#define ADVERTISE_MANDATORY_DATA_TYPE_MANUFACTURER  (0xFF)

#define ADVERTISE_COMPANY_ID                        (0x0047) /* Silicon Labs */
#define ADVERTISE_FIRMWARE_ID                       (0x0000)

/** Complete local name. */
#define ADVERTISE_TYPE_LOCAL_NAME                   (0x09)
#define ADVERTISE_DEVICE_NAME_LEN                   (15)
#define ADVERTISE_DEVICE_NAME                       "pH Pond Monitor"

// -------------------------------
// Structure that holds Scan Response data
typedef struct {
  uint8_t flags_length;          /**< Length of the Flags field. */
  uint8_t flags_type;            /**< Type of the Flags field. */
  uint8_t flags;                 /**< Flags field. */
  uint8_t mandatory_data_length; /**< Length of the mandata field. */
  uint8_t mandatory_data_type;   /**< Type of the mandata field. */
  uint8_t company_id[2];         /**< Company ID. */
  uint8_t firmware_id[2];        /**< Firmware ID */
  uint8_t local_name_length;     /**< Length of the local name field. */
  uint8_t local_name_type;       /**< Type of the local name field. */
  uint8_t local_name[ADVERTISE_DEVICE_NAME_LEN]; /**< Local name field. */
} advertise_scan_response_t;

#define ADVERTISE_SCAN_RESPONSE_DEFAULT                                \
  {                                                                    \
    .flags_length = ADVERTISE_FLAGS_LENGTH,                            \
    .flags_type = ADVERTISE_FLAGS_TYPE,                                \
    .flags = ADVERTISE_FLAGS_LE_GENERAL_DISCOVERABLE                   \
             | ADVERTISE_FLAGS_BR_EDR_NOT_SUPPORTED,                   \
    .mandatory_data_length = ADVERTISE_MANDATORY_DATA_LENGTH,          \
    .mandatory_data_type = ADVERTISE_MANDATORY_DATA_TYPE_MANUFACTURER, \
    .company_id = UINT16_TO_BYTES(ADVERTISE_COMPANY_ID),               \
    .firmware_id = UINT16_TO_BYTES(ADVERTISE_FIRMWARE_ID),             \
    .local_name_length = ADVERTISE_DEVICE_NAME_LEN + 1,                \
    .local_name_type = ADVERTISE_TYPE_LOCAL_NAME,                      \
    .local_name = ADVERTISE_DEVICE_NAME                                \
  }

/** Helper macro */
#define UINT16_TO_BYTES(x) { (uint8_t)(x), (uint8_t)((x) >> 8) }

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
static const advertise_scan_response_t adv_scan_response
  = ADVERTISE_SCAN_RESPONSE_DEFAULT;
static float pH_value = NEUTRAL_PH_VALUE;
extern uint8_t calibrated;

static void app_pH_value_process_evt_gatt_server_user_read_request(
  sl_bt_evt_gatt_server_user_read_request_t *data);

// Application Init.
void app_init(void)
{
  app_sparkfun_micro_oled_init();
  app_mikroe_ph2_init();
}

// Application Process Action.
void app_process_action(void)
{
  if (app_is_process_required()) {
    if (!calibrated) {
      sl_bt_nvm_save(CALIBRATED_NVM_ADDRESS,
                     sizeof(calibrated),
                     &calibrated);
      app_mikroe_ph2_calibrate();
    }

    app_mikroe_ph2_process(&pH_value);
    app_sparkfun_micro_oled_process(pH_value);
  }
  app_proceed();
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
  bd_addr address;
  uint8_t address_type;
  uint8_t system_id[8];

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:

      // Extract unique ID from BT Address.
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      app_assert_status(sc);

      // Pad and reverse unique ID to get System ID.
      system_id[0] = address.addr[5];
      system_id[1] = address.addr[4];
      system_id[2] = address.addr[3];
      system_id[3] = 0xFF;
      system_id[4] = 0xFE;
      system_id[5] = address.addr[2];
      system_id[6] = address.addr[1];
      system_id[7] = address.addr[0];

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
                                                   0,
                                                   sizeof(system_id),
                                                   system_id);
      app_assert_status(sc);
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

      // Set custom advertising payload
      sc = sl_bt_legacy_advertiser_set_data(advertising_set_handle,
                                            sl_bt_advertiser_scan_response_packet,
                                            sizeof(adv_scan_response),
                                            (uint8_t *)&adv_scan_response);
      app_assert_status(sc);

      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);
      app_log("Started advertising...\r\n");

      // Maximum allowed bonding count: 8
      // New bonding will overwrite the bonding that was used the longest time
      // ago
      sc = sl_bt_sm_store_bonding_configuration(8, 0x2);
      app_assert_status(sc);

      // Capabilities: No Input and No Output
      sc =
        sl_bt_sm_configure(0b00000010, sl_bt_sm_io_capability_noinputnooutput);
      app_assert_status(sc);

      // Allow bondings
      sc = sl_bt_sm_set_bondable_mode(1);
      app_assert_status(sc);

      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      sl_status_t sc;

      app_log("Bluetooth Stack Event : CONNECTION OPENED\r\n");

      sc = sl_bt_advertiser_stop(advertising_set_handle);
      app_assert_status(sc);
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      app_log("Bluetooth Stack Event : CONNECTION CLOSED\r\n");

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    case sl_bt_evt_gatt_server_user_read_request_id:
      app_pH_value_process_evt_gatt_server_user_read_request(
        &(evt->data.evt_gatt_server_user_read_request));
      break;

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

static void app_pH_value_process_evt_gatt_server_user_read_request(
  sl_bt_evt_gatt_server_user_read_request_t *data)
{
  char temp_string[20] = { 0 };
  int len = snprintf(NULL, 0, "%.2f", pH_value);
  snprintf(temp_string, len + 1, "%.2f", pH_value);

  if (data->characteristic == gattdb_pH_value) {
    sl_bt_gatt_server_send_user_read_response(data->connection,
                                              data->characteristic,
                                              0,
                                              len + 1,
                                              (uint8_t *)temp_string,
                                              NULL);
  }
}

void sl_button_on_change(const sl_button_t *handle)
{
  if ((sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED)
      && (SL_SIMPLE_BUTTON_INSTANCE(0) == handle)) {
    calibrated = false;
  }
}
