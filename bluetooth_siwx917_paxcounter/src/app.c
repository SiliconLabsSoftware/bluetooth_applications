/***************************************************************************//**
 * @file
 * @brief Core application logic.
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

/**
 * Include files
 * */
//! SL Wi-Fi SDK includes
#include "sl_board_configuration.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "cmsis_os2.h"
#include "sl_utility.h"
#include "FreeRTOSConfig.h"
//! BLE include file to refer BLE APIs
#include <string.h>

#include "ble_config.h"
#include "rsi_ble.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common.h"
#include "rsi_bt_common_apis.h"
#include "rsi_common_apis.h"

#include "pax_nvm.h"

#include "sl_assert.h"

// Signal strength threshold (in dBm) used to filter out distant or weak devices.
#define RSSI_THRESHOLD         -50

// Time in milliseconds to perform periodic PAX (occupancy) analysis and output the current count.
#define PAX_UPDATE_TIMEOUT     5000

// Application supported events list
#define APP_EVENT_ADV_REPORT   0
#define APP_EVENT_ANALYZING    1

// Store device adv data
device_id_t device_id;

// Update timer handle
osTimerId_t update_timer_handle;

// Timer Attributes
const osTimerAttr_t update_timer = {
  .name = "update_timer",
  .attr_bits = 0,
};

// Scanner state
bool scanner_state = 0;

// BLE task semaphores & event map variables
osSemaphoreId_t ble_main_task_sem;
static volatile uint32_t ble_app_event_map;
static volatile uint32_t ble_app_event_map1;

static const sl_wifi_device_configuration_t
  ble_config = { .boot_option = LOAD_NWP_FW,
                 .mac_address = NULL,
                 .band = SL_SI91X_WIFI_BAND_2_4GHZ,
                 .region_code = US,
                 .boot_config = {
                   .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_BLE_MODE,
                   .feature_bit_map = (SL_SI91X_FEAT_WPS_DISABLE
                                       | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
                                       | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1),
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT
                      | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map =
                     (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID),
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_LOW_POWER_MODE
                      | SL_SI91X_EXT_FEAT_XTAL_CLK
                      | MEMORY_CONFIG
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
                      |
                      SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE                           // Enable BT feature
                     ),
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   .bt_feature_bit_map = (SL_SI91X_BT_RF_TYPE
                                          | SL_SI91X_ENABLE_BLE_PROTOCOL),

                   /*Enable BLE custom feature bitmap*/
                   .ble_feature_bit_map =
                     (SL_SI91X_BLE_MAX_NBR_PERIPHERALS(
                        RSI_BLE_MAX_NBR_PERIPHERALS)
                      | SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS)
                      | SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV)
                      | SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC)
                      | SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID
                      | SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX)
                      | SL_SI91X_BLE_PWR_SAVE_OPTIONS(
                        RSI_BLE_PWR_SAVE_OPTIONS)
                      | SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if RSI_BLE_GATT_ASYNC_ENABLE
                      | SL_SI91X_BLE_GATT_ASYNC_ENABLE
#endif
                     ),
                   .ble_ext_feature_bit_map =
                     (SL_SI91X_BLE_NUM_CONN_EVENTS(RSI_BLE_NUM_CONN_EVENTS)
                      | SL_SI91X_BLE_NUM_REC_BYTES(RSI_BLE_NUM_REC_BYTES)
#if RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
                      | SL_SI91X_BLE_INDICATE_CONFIRMATION_FROM_HOST // indication response from app
#endif
#if RSI_BLE_MTU_EXCHANGE_FROM_HOST
                      | SL_SI91X_BLE_MTU_EXCHANGE_FROM_HOST // MTU Exchange request initiation from app
#endif
#if RSI_BLE_SET_SCAN_RESP_DATA_FROM_HOST
                      | (SL_SI91X_BLE_SET_SCAN_RESP_DATA_FROM_HOST) // Set SCAN Resp Data from app
#endif
#if RSI_BLE_DISABLE_CODED_PHY_FROM_HOST
                      | (SL_SI91X_BLE_DISABLE_CODED_PHY_FROM_HOST) // Disable Coded PHY from app
#endif
#if BLE_SIMPLE_GATT
                      | SL_SI91X_BLE_GATT_INIT
#endif
                     ),
                   .config_feature_bit_map =
                     (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP
                      | SL_SI91X_ENABLE_ENHANCED_MAX_PSP)
                 } };

const osThreadAttr_t thread_attributes = {
  .name = "application_thread",
  .attr_bits = 0,
  .cb_mem = 0,
  .cb_size = 0,
  .stack_mem = 0,
  .stack_size = 3072,
  .priority = osPriorityNormal,
  .tz_module = 0,
  .reserved = 0,
};

// Initializes the event parameter. This function is used during BLE initialization.
static void rsi_ble_app_init_events()
{
  ble_app_event_map = 0;
  ble_app_event_map1 = 0;
  return;
}

// Set a specific BLE event. This function is used to set/raise the specific event.
void rsi_ble_app_set_event(uint32_t event_num)
{
  if (event_num < 32) {
    ble_app_event_map |= BIT(event_num);
  } else {
    ble_app_event_map1 |= BIT((event_num - 32));
  }

  osSemaphoreRelease(ble_main_task_sem);

  return;
}

// Clear a specific BLE event. This function is used to clear the specific event.
static void rsi_ble_app_clear_event(uint32_t event_num)
{
  if (event_num < 32) {
    ble_app_event_map &= ~BIT(event_num);
  } else {
    ble_app_event_map1 &= ~BIT((event_num - 32));
  }

  return;
}

// Returns the first set event based on priority. Returns the highest priority event among all the set events
static int32_t rsi_ble_app_get_event(void)
{
  uint32_t ix;

  for (ix = 0; ix < 64; ix++) {
    if (ix < 32) {
      if (ble_app_event_map & (1 << ix)) {
        return ix;
      }
    } else {
      if (ble_app_event_map1 & (1 << (ix - 32))) {
        return ix;
      }
    }
  }

  return (-1);
}

/******************************************************
 *          Callback Function Definitions
 ******************************************************/
// Timer callback function
void update_timer_callback()
{
  rsi_ble_app_set_event(APP_EVENT_ANALYZING);
}

// Advertisement Report Callback
void rsi_ble_simple_central_on_adv_report_event(
  rsi_ble_event_adv_report_t *adv_report)
{
  if (adv_report->rssi < RSSI_THRESHOLD) {
    return;
  }

  rsi_6byte_dev_address_to_ascii(device_id.bd_ascii_addr,
                                 (uint8_t *)adv_report->dev_addr);

  device_id.bd_addr_type = adv_report->dev_addr_type;

  memcpy(device_id.adv_data, adv_report->adv_data, adv_report->adv_data_len);

  device_id.adv_data_len = adv_report->adv_data_len;

  device_id.last_seen = osKernelGetTickCount();

  rsi_ble_app_set_event(APP_EVENT_ADV_REPORT);
}

// Check and restart scanner if needed
sl_status_t restart_scanner_if_needed(bool *scanner_state)
{
  sl_status_t sc;
  if (*scanner_state) {
    sc = rsi_ble_stop_scanning();
    if (sc == SL_STATUS_OK) {
      *scanner_state = false;
    }
    return sc;
  }

  sc = rsi_ble_start_scanning();
  if (sc == SL_STATUS_OK) {
    *scanner_state = true;
  }
  return sc;
}

// Check if device data already exists in NVM
bool device_exists_in_nvm(uint32_t hash, const char *addr)
{
  uint8_t temp_addr[18];
  uint32_t temp_time;
  sl_status_t sc = nvm3_read(hash, temp_addr, &temp_time);

  return (sc == ECODE_NVM3_OK && strcmp((const char *)temp_addr, addr) == 0);
}

// Add new device to NVM or update last seen value or perform linear probing if existing device
void handle_new_or_existing_device(uint32_t hash, device_id_t *device)
{
  char *bd_addr = (char *)device->bd_ascii_addr;

  for (uint32_t i = 0; i < MAX_OBJECT_COUNT; ++i) {
    uint32_t try_hash = (hash + i) % MAX_OBJECT_COUNT;

    if (device_exists_in_nvm(try_hash, bd_addr)) {
      device->last_seen = osKernelGetTickCount();
      nvm3_add_device(try_hash, device, sizeof(device_id_t));
      return;
    }

    sl_status_t sc = nvm3_read(try_hash, NULL, NULL);
    if (sc == ECODE_NVM3_ERR_READ_FAILED) {
      nvm3_add_device(try_hash, device, sizeof(device_id_t));
      nvm3_incrementCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, NULL);
      return;
    }
  }
}

void app_process(void *argument)
{
  UNUSED_PARAMETER(argument);

  int32_t status = 0;
  int32_t temp_event_map = 0;
  sl_wifi_firmware_version_t version = { 0 };
  static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN] = { 0 };
  uint8_t local_dev_addr[BD_ADDR_LEN] = { 0 };

  // Init Wi-Fi
  status = sl_wifi_init(&ble_config, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
    printf("\r\nWi-Fi Initialization Failed, Error Code : 0x%lX\r\n", status);
  } else {
    printf("\r\nWi-Fi Initialization Success\n");
  }

  // Firmware version Prints
  status = sl_wifi_get_firmware_version(&version);
  if (status != SL_STATUS_OK) {
    printf("\r\nFirmware version Failed, Error Code : 0x%lX\r\n", status);
  } else {
    print_firmware_version(&version);
  }

  // Init NVM
  status = nvm3_initDefault();
  printf("\r\nNVM3 init status %lx \r\n", status);

  // Erase all NVM3 data on boot-up
  nvm3_eraseAll(NVM3_DEFAULT_HANDLE);

  // Initialise the counter objects to track writes and deletes.
  initialise_counters();

  // Get the local device MAC address.
  status = rsi_bt_get_local_device_address(rsi_app_resp_get_dev_addr);
  if (status != SL_STATUS_OK) {
    printf("\r\nGet local device address failed = %lx\r\n", status);
  } else {
    rsi_6byte_dev_address_to_ascii(local_dev_addr, rsi_app_resp_get_dev_addr);
    printf("\r\nLocal device address %s \r\n", local_dev_addr);
  }

  // BLE register GAP callbacks
  rsi_ble_gap_register_callbacks(rsi_ble_simple_central_on_adv_report_event,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);

  ble_main_task_sem = osSemaphoreNew(1, 0, NULL);
  if (ble_main_task_sem == NULL) {
    printf("Failed to create ble_main_task_sem semaphore\r\n");
  }

  // initialize the event map
  rsi_ble_app_init_events();

  // Start scanning
  status = rsi_ble_start_scanning();
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to Start Scanner: 0x%lX\r\n", status);
  } else {
    scanner_state = 1;
  }

  // Initialize the update timer
  update_timer_handle = osTimerNew(&update_timer_callback,
                                   osTimerPeriodic,
                                   NULL,
                                   &update_timer);

  // Start timer to perform data update procedure every PAX_UPDATE_TIME
  osTimerStart(update_timer_handle, PAX_UPDATE_TIMEOUT);

  while (1) {
    // Application main loop
    // checking for received events
    temp_event_map = rsi_ble_app_get_event();

    if (temp_event_map == RSI_FAILURE) {
      osSemaphoreAcquire(ble_main_task_sem, osWaitForever);
      continue;
    }

    switch (temp_event_map) {
      case APP_EVENT_ADV_REPORT: {
        // advertise report event.

        // clear the advertise report event.
        rsi_ble_app_clear_event(APP_EVENT_ADV_REPORT);

        restart_scanner_if_needed(&scanner_state);

        uint32_t hash = rokkit_hash((char *)&device_id.bd_ascii_addr,
                                    18) % MAX_OBJECT_COUNT;

        handle_new_or_existing_device(hash, &device_id);

        restart_scanner_if_needed(&scanner_state);
      }
      break;

      case APP_EVENT_ANALYZING:

        printf("\r\nAnalyzing Data\r\n");

        restart_scanner_if_needed(&scanner_state);

        uint32_t pax_count;

        nvm3_update(&pax_count);

        printf("BLE PAC Count : %lu \r\n\r\n", pax_count);

        restart_scanner_if_needed(&scanner_state);

        // Clear the analyze report event
        rsi_ble_app_clear_event(APP_EVENT_ANALYZING);

        break;

      default: {
      }
    }
  }
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)app_process, NULL, &thread_attributes);
}
