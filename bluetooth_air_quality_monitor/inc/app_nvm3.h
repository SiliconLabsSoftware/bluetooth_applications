/***************************************************************************//**
 * @file app_nvm3.h
 * @brief NVM3 application header
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

#ifndef NVM3_USER_H
#define NVM3_USER_H

#include "sl_status.h"
#include "nvm3_default.h"
#include "app_air_quality.h"

/***************************************************************************//**
 * @addtogroup nvm3_user
 * @brief nvm3_user interface.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *  Initialize the NVM3.
 *
 ******************************************************************************/
sl_status_t nvm3_user_init(air_quality_config_t *config);

/***************************************************************************//**
 * @brief
 *  Set the alarm enabled to NVM.
 *
 * @param[in] enable
 *   Status of alarm.
 *
 * @return
 *   @ref SL_STATUS_OK  on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_set_alarm_enabled(uint8_t enable);

/***************************************************************************//**
 * @brief
 *  Get the alarm status from NVM.
 *
 * @param[out] enable
 *   Status of alarm.
 *
 * @return
 *   @ref SL_STATUS_OK  on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_get_alarm_enabled(uint8_t *enable);

/***************************************************************************//**
 * @brief
 *  Set the Measurement interval in seconds value to NVM.
 *
 * @param[in] interval
 *   Data to write.
 *
 * @return
 *   @ref SL_STATUS_OK  on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_set_measurement_interval(uint8_t interval);

/***************************************************************************//**
 * @brief
 *  Get the Measurement interval in seconds value from NVM.
 *
 * @param[out] interval
 *   Data to read.
 *
 * @return
 *   @ref SL_STATUS_OK  on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_get_measurement_interval(uint8_t *interval);

/***************************************************************************//**
 * @brief
 *  Set the buzzer volume value to NVM.
 *
 * @param[in] volume
 *   Data to write.
 *
 * @return
 *   @ref SL_STATUS_OK  on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_set_buzzer_volume(uint8_t volume);

/***************************************************************************//**
 * @brief
 *  Get the buzzer volume from NVM.
 *
 * @param[out] volume
 *   The Volume value. It should be in [0:10] range.
 *
 * @return
 *   @ref SL_STATUS_OK  on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_get_buzzer_volume(uint8_t *volume);

/***************************************************************************//**
 * @brief
 *  Set the alarm threshold for CO2 level in ppm to NVM.
 *
 * @param[in] threshold
 *   Data to write.
 *
 * @return
 *   @ref SL_STATUS_OK  on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_set_co2_threshold(uint16_t threshold);

/***************************************************************************//**
 * @brief
 *  Get the alarm threshold for CO2 level in ppm from NVM.
 *
 * @param[out] threshold
 *   The alarm threshold for CO2 level in ppm.
 *
 * @return
 *   @ref SL_STATUS_OK  on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_get_co2_threshold(uint16_t *threshold);

/***************************************************************************//**
 * @brief
 *  Set the alarm threshold for tVOC level in ppb to NVM.
 *
 * @param[in] threshold
 *   Data to write.
 *
 * @return
 *   @ref SL_STATUS_OK  on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_set_tvoc_threshold(uint16_t threshold);

/***************************************************************************//**
 * @brief
 *  Get the alarm threshold for tVOC level in ppb from NVM.
 *
 * @param[out] threshold
 *   The alarm threshold for CO2 level in ppm.
 *
 * @return
 *   @ref SL_STATUS_OK  on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_get_tvoc_threshold(uint16_t *threshold);

/** @} */

#endif // NVM3_USER_H
