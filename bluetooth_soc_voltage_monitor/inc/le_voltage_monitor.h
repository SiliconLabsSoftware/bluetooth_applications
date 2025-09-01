/***************************************************************************//**
 * @file le_voltage_monitor.h
 * @brief Voltage measuring interface
 * @version 1.0
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef LE_VOLTAGE_MONITOR_H_
#define LE_VOLTAGE_MONITOR_H_

#include <stdint.h>

/***************************************************************************//**
 * @brief
 *    External signal bit mask for the BLE API
 ******************************************************************************/
#define _BIT(x)                          ((0x01) << x)
#define LE_MONITOR_SIGNAL                 (_BIT(0))
#define LE_PRS_FEATURE_CONFIG_SIGNAL      (_BIT(1))

/***************************************************************************//**
 * @brief
 *    Number of samples to measure before calculating the average and notifying
 *    the connected device.
 ******************************************************************************/
#define NUM_OF_SAMPLES                    (128)

/***************************************************************************//**
 * @brief
 *    Sampling frequency of the voltage reading. (Frequency that device will be
 *    notified)
 ******************************************************************************/
#define SAMPLING_FREQ_HZ                  (50)

/***************************************************************************//**
 * @brief
 *    Initialize the low energy peripherals to measure the supply voltage.
 *
 * @details
 *    The LETIMER, PRS, IADC, and LDMA peripherals are initialized. The
 *    LETIMER's underflow event will be connected to the IADC start conversion
 *    trigger through PRS. The LDMA will transfer the data to a buffer when the
 *    IADC conversion is complete.
 *
 * @note
 *    The LDMA will not begin transferring the data after initialization. The
 *    le_voltage_monitor_start_next() function must be called to start
 *    measuring data.
 ******************************************************************************/
void le_voltage_monitor_init(void);

/***************************************************************************//**
 * @brief
 *    Gets the average millivoltage of the samples taken between complete LDMA
 *    transfers.
 *
 * @return
 *    Average voltage in millivolts
 ******************************************************************************/
uint16_t le_voltage_monitor_get_average_mv(void);

/***************************************************************************//**
 * @brief
 *    Starts the peripherals to begin sampling until internal buffer is filled.
 ******************************************************************************/
void le_voltage_monitor_start_next(void);

/***************************************************************************//**
 * @brief
 *    Stops the sampling.
 ******************************************************************************/
void le_voltage_monitor_stop(void);

/***************************************************************************//**
 * @brief
 *    le_voltage_monitor_conifg_prs_feature.
 ******************************************************************************/
void le_voltage_monitor_conifg_prs_feature(void);

#endif /* LE_VOLTAGE_MONITOR_H_ */
