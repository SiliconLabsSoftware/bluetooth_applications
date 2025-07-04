/***************************************************************************//**
 * @file ble_config.h
 * @brief ble config header file.
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

#ifndef BLE_CONFIG_H
#define BLE_CONFIG_H

/******************************************************
 * *                      Macros
 * ******************************************************/
#define RSI_BLE_SET_RAND_ADDR                   "04:D4:C4:9A:F3:CC"

#define CLEAR_ACCEPTLIST                        0x00
#define ADD_DEVICE_TO_ACCEPTLIST                0x01
#define DELETE_DEVICE_FROM_ACCEPTLIST           0x02

#define ALL_PHYS                                0x00

#define RSI_BLE_DEV_ADDR_RESOLUTION_ENABLE      0

#define RSI_OPERMODE_WLAN_BLE                   13

#ifdef SLI_SI91X_MCU_INTERFACE
#define RSI_BLE_MAX_NBR_ATT_REC                 20
#define RSI_BLE_MAX_NBR_PERIPHERALS             1
#define RSI_BLE_MAX_NBR_CENTRALS                1
#define RSI_BLE_NUM_CONN_EVENTS                 2
#else
#define RSI_BLE_MAX_NBR_ATT_REC                 80
#define RSI_BLE_MAX_NBR_PERIPHERALS             3
#define RSI_BLE_MAX_NBR_CENTRALS                1
#define RSI_BLE_NUM_CONN_EVENTS                 20
#endif

#define RSI_BLE_MAX_NBR_ATT_SERV                10
#define RSI_BLE_GATT_ASYNC_ENABLE               0
#define RSI_BLE_GATT_INIT                       0

/* Number of BLE GATT RECORD SIZE IN (n*16 BYTES), eg:(0x40*16)=1024 bytes */
#define RSI_BLE_NUM_REC_BYTES                   0x40

#define RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST 0

/*=======================================================================*/
//! Advertising command parameters

/*=======================================================================*/

#define RSI_BLE_ADV_TYPE                                UNDIR_CONN
#define RSI_BLE_ADV_FILTER_TYPE \
  ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY
#define RSI_BLE_ADV_DIR_ADDR_TYPE                       LE_PUBLIC_ADDRESS
#define RSI_BLE_ADV_DIR_ADDR                            "00:15:83:6A:64:17"

#define RSI_BLE_ADV_INT_MIN                             0x100
#define RSI_BLE_ADV_INT_MAX                             0x200
#define RSI_BLE_ADV_CHANNEL_MAP                         0x07

//! Advertise status
//! Start the advertising process
#define RSI_BLE_START_ADV                               0x01
//! Stop the advertising process
#define RSI_BLE_STOP_ADV                                0x00

//! BLE Tx Power Index On Air
#ifdef SLI_SI915
#define RSI_BLE_PWR_INX                                 75 //  HP chain for Si915
#else
#define RSI_BLE_PWR_INX                                 30
#endif
//! BLE Active H/w Pwr Features
#define BLE_DISABLE_DUTY_CYCLING                        0
#define BLE_DUTY_CYCLING                                1
#define BLR_DUTY_CYCLING                                2
#define BLE_4X_PWR_SAVE_MODE                            4
#define RSI_BLE_PWR_SAVE_OPTIONS                        BLE_DISABLE_DUTY_CYCLING

//! Advertise types

/* Advertising will be visible(discoverable) to all the devices.
 * Scanning/Connection is also accepted from all devices
 * */
#define UNDIR_CONN                                      0x80

/* Advertising will be visible(discoverable) to the particular device
 * mentioned in RSI_BLE_ADV_DIR_ADDR only.
 * Scanning and Connection will be accepted from that device only.
 * */
#define DIR_CONN                                        0x81

/* Advertising will be visible(discoverable) to all the devices.
 * Scanning will be accepted from all the devices.
 * Connection will be not be accepted from any device.
 * */
#define UNDIR_SCAN                                      0x82

/* Advertising will be visible(discoverable) to all the devices.
 * Scanning and Connection will not be accepted from any device
 * */
#define UNDIR_NON_CONN                                  0x83

/* Advertising will be visible(discoverable) to the particular device
 * mentioned in RSI_BLE_ADV_DIR_ADDR only.
 * Scanning and Connection will be accepted from that device only.
 * */
#define DIR_CONN_LOW_DUTY_CYCLE                         0x84

//! Advertising flags
#define LE_LIMITED_DISCOVERABLE                         0x01
#define LE_GENERAL_DISCOVERABLE                         0x02
#define LE_BR_EDR_NOT_SUPPORTED                         0x04

//! Advertise filters
#define ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY                 0x00
#define ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ANY         0x01
#define ALLOW_SCAN_REQ_ANY_CONN_REQ_ACCEPT_LIST         0x02
#define ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ACCEPT_LIST 0x03

//! Address types
#define LE_PUBLIC_ADDRESS                               0x00
#define LE_RANDOM_ADDRESS                               0x01
#define LE_RESOLVABLE_PUBLIC_ADDRESS                    0x02
#define LE_RESOLVABLE_RANDOM_ADDRESS                    0x03

/*=======================================================================*/

/*=======================================================================*/
//! Connection parameters

/*=======================================================================*/
#define LE_SCAN_INTERVAL        0x0100
#define LE_SCAN_WINDOW          0x0050

#define CONNECTION_INTERVAL_MIN 0x00A0
#define CONNECTION_INTERVAL_MAX 0x00A0

#define CONNECTION_LATENCY      0x0000
#define SUPERVISION_TIMEOUT     0x07D0 // 2000

/*=======================================================================*/

/*=======================================================================*/
//! Scan command parameters

/*=======================================================================*/

#define RSI_BLE_SCAN_TYPE                 SCAN_TYPE_PASSIVE
#define RSI_BLE_SCAN_FILTER_TYPE          SCAN_FILTER_TYPE_ALL

//! Scan status
#define RSI_BLE_START_SCAN                0x01
#define RSI_BLE_STOP_SCAN                 0x00

//! Scan types
#define SCAN_TYPE_ACTIVE                  0x01
#define SCAN_TYPE_PASSIVE                 0x00

//! Scan filters
#define SCAN_FILTER_TYPE_ALL              0x00
#define SCAN_FILTER_TYPE_ONLY_ACCEPT_LIST 0x01

#define RSI_SEL_INTERNAL_ANTENNA          0x00
#define RSI_SEL_EXTERNAL_ANTENNA          0x01

/*=======================================================================*/
//! Parameters for acceptlisting of BLE advertise reports based on the ADV payload

/*=======================================================================*/
// #define RSI_BLE_ENABLE_ACCEPTLIST_BASEDON_ADV_PAYLOAD     0x00
// #define RSI_BLE_ADV_PAYLOAD_INDEX_FOR_COMPARE            0x00
// #define RSI_BLE_ADV_PAYLOAD_LENGTH_FROM_INDEX_TO_COMPARE 0x00

#endif
