/***************************************************************************//**
 * @file pax_nvm.h
 * @brief pax counter nvm header file.
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
 * 1. The origin of this software must not be misrepresented{} you must not
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

#ifndef PAX_NVM_H_
#define PAX_NVM_H_

#include "sl_utility.h"
#include <stdint.h>
#include <string.h>

#include "stdio.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "cmsis_compiler.h"             // Compiler agnostic definitions
#include "os_tick.h"                    // OS Tick API

#include "nvm3_default.h"
#include "nvm3_default_config.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define NVM3_DEFAULT_HANDLE  nvm3_defaultHandle

// Maximum number of data objects saved
#define MAX_OBJECT_COUNT     126

// Max and min keys for data objects
#define MIN_DATA_KEY         NVM3_KEY_MIN
#define MAX_DATA_KEY         (MIN_DATA_KEY + MAX_OBJECT_COUNT - 1)

// Key of write counter object
#define WRITE_COUNTER_KEY    MAX_OBJECT_COUNT

// Key of delete counter object
#define DELETE_COUNTER_KEY   (WRITE_COUNTER_KEY + 1)

// Time in **milliseconds** before a MAC address is considered to have "left the room."
#define OCCUPANCY_TIMEOUT_MS 60000

// Length of the device address
#define BD_ADDR_LEN          18

typedef struct device_id_s{
  // BLE Address stored in ASCII format
  // Use APIs rsi_ascii_mac_address_to_6bytes & rsi_6byte_dev_address_to_ascii to convert MAC from ASCII to HEX and vice versa
  uint8_t bd_ascii_addr[18];
  // BLE Address type
  uint8_t bd_addr_type;
  // Advertisement data
  uint8_t adv_data[31];
  // Length of Advertisment data
  uint8_t adv_data_len;
  // Last seen ticks
  uint32_t last_seen;
}device_id_t;

// Hash function to generate a 32bit has using the ASCII BLE Address
uint32_t rokkit_hash (const char *data, uint16_t len);

// Initialize the Write & Delete counters in NVM
void initialise_counters(void);

// Add/Update device data in NVM
sl_status_t nvm3_add_device(uint32_t key, device_id_t *data, uint32_t len);

// Read device data in NVM
sl_status_t nvm3_read(nvm3_ObjectKey_t key,
                      uint8_t bd_ascii_addr[18],
                      uint32_t *last_seen);

// Read NVM data and perfrom PAX analysis
void nvm3_update(uint32_t *pax_count);

// Delete device data in NVM
sl_status_t nvm3_app_delete(uint32_t key);

#endif /* PAX_NVM_H_ */
