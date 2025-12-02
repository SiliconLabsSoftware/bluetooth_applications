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

#ifndef APP_NVM3_USER_H
#define APP_NVM3_USER_H

#include "sl_status.h"
#include "app_thermostat.h"

/***************************************************************************//**
 * @brief
 *    Initialize the nvm3 for user configuration.
 *
 ******************************************************************************/
sl_status_t nvm3_user_init(thermostat_config_t *config);

/***************************************************************************//**
 * @brief
 *    Set mode to the nvm3 entry.
 *
 * @param[in] mode
 *    Mode to set.
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_set_mode(uint8_t mode);

/***************************************************************************//**
 * @brief
 *    Get mode value from the nvm3 entry.
 *
 * @param[in] mode
 *    Mode to get.
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_get_mode(uint8_t *mode);

/***************************************************************************//**
 * @brief
 *    Set setpoint value to the nvm3 entry.
 *
 * @param[in] setpoint
 *    Setpoint value
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_set_setpoint(int16_t setpoint);

/***************************************************************************//**
 * @brief
 *    Get setpoint value from the nvm3 entry.
 *
 * @param[in] setpoint
 *    Setpoint value
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_get_setpoint(int16_t *setpoint);

/***************************************************************************//**
 * @brief
 *    Set hysteresis value to the nvm3 entry.
 *
 * @param[in] hysteresis
 *    Hysteresis value
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_set_hysteresis(int16_t hysteresis);

/***************************************************************************//**
 * @brief
 *    Get hysteresis value from the nvm3 entry.
 *
 * @param[in] hysteresis
 *    Hysteresis value
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_get_hysteresis(int16_t *hysteresis);

/***************************************************************************//**
 * @brief
 *    Set lower threshold value to the nvm3 entry.
 *
 * @param[in] threshold
 *    threshold value
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_set_lower_threshold(int16_t threshold);

/***************************************************************************//**
 * @brief
 *    Get lower threshold value from the nvm3 entry.
 *
 * @param[in] threshold
 *    threshold value
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_get_lower_threshold(int16_t *threshold);

/***************************************************************************//**
 * @brief
 *    Set upper threshold value to the nvm3 entry.
 *
 * @param[in] threshold
 *    threshold value
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_set_upper_threshold(int16_t threshold);

/***************************************************************************//**
 * @brief
 *    Get upper threshold value from the nvm3 entry.
 *
 * @param[in] threshold
 *    threshold value
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_get_upper_threshold(int16_t *threshold);

/***************************************************************************//**
 * @brief
 *    Set state of alarm enabled to the nvm3 entry.
 *
 * @param[out] enable
 *   State of alarm enabled.
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_set_alarm_enabled(uint8_t enable);

/***************************************************************************//**
 * @brief
 *    Get state of alarm enabled from the nvm3 entry.
 *
 * @param[out] enable
 *   State of alarm enabled.
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_get_alarm_enabled(uint8_t *enable);

/***************************************************************************//**
 * @brief
 *  Set the Measurement Interval in seconds to NVM.
 *
 * @param[in] interval
 *   Measurement interval value. It should be in [1:30] range.
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_set_measurement_interval(uint8_t interval);

/***************************************************************************//**
 * @brief
 *  Get the Measurement Interval in seconds from NVM.
 *
 * @param[out] interval
 *   Measurement interval value. It should be in [1:30] range.
 *
 * @return
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
sl_status_t nvm3_user_get_measurement_interval(uint8_t *interval);

/***************************************************************************//**
 *  Set the Buzzer Volume to NVM.
 ******************************************************************************/
sl_status_t nvm3_user_set_buzzer_volume(uint8_t volume);

/***************************************************************************//**
 *  Get the Buzzer Volume from NVM.
 ******************************************************************************/
sl_status_t nvm3_user_get_buzzer_volume(uint8_t *volume);

#endif // APP_NVM3_USER_H
