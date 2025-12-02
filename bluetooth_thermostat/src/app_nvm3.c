/***************************************************************************//**
 * @file app_thermostat_nvm3.c
 * @brief NVM3 application code
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
#include "nvm3_default_config.h"
#include "nvm3_default.h"
#include "app_nvm3.h"

/***************************************************************************//**
 * @addtogroup user_config_nvm3
 * @brief  NVM3 User configuration.
 * @details
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Defines

// Max and min keys for data objects
#define MODE_KEY                       (NVM3_KEY_MIN)
#define SETPOINT_KEY                   (NVM3_KEY_MIN + 1)
#define HYSTERESIS_KEY                 (NVM3_KEY_MIN + 2)
#define LOWER_THRESHOLD_KEY            (NVM3_KEY_MIN + 3)
#define UPPER_THRESHOLD_KEY            (NVM3_KEY_MIN + 4)
#define ALARM_ENABLED_KEY              (NVM3_KEY_MIN + 5)
#define MEASUREMENT_INTERVAL_KEY       (NVM3_KEY_MIN + 6)
#define BUZZER_VOLUME_KEY              (NVM3_KEY_MIN + 7)

#define MODE_DEFAULT                   0

#define ALARM_ENABLED_DEFAULT          1

#define SETPOINT_MIN                   (-350)
#define SETPOINT_MAX                   1200
#define SETPOINT_DEFAULT               250

#define HYSTERESIS_MIN                 0
#define HYSTERESIS_MAX                 1550
#define HYSTERESIS_DEFAULT             20

#define LOWER_THRESHOLD_MIN            (-350)
#define LOWER_THRESHOLD_MAX            1200
#define LOWER_THRESHOLD_DEFAULT        0

#define UPPER_THRESHOLD_MIN            (-350)
#define UPPER_THRESHOLD_MAX            1200
#define UPPER_THRESHOLD_DEFAULT        500

#define MEASUREMENT_INTERVAL_MIN       1
#define MEASUREMENT_INTERVAL_MAX       30
#define MEASUREMENT_INTERVAL_DEFAULT   5

#define BUZZER_VOLUME_MIN              1
#define BUZZER_VOLUME_MAX              30
#define BUZZER_VOLUME_DEFAULT          5

// Use the default nvm3 handle from nvm3_default.h
#define NVM3_DEFAULT_HANDLE            nvm3_defaultHandle

// -----------------------------------------------------------------------------
// Public function definitions

/***************************************************************************//**
 * Initialize NVM3 user config.
 ******************************************************************************/
sl_status_t nvm3_user_init(thermostat_config_t *config)
{
  // This will call nvm3_open() with default parameters for
  // memory base address and size, cache size, etc.
  sl_status_t stt = nvm3_initDefault();
  if (stt != SL_STATUS_OK) {
    return stt;
  }

  // Initialize the mode config.
  if (SL_STATUS_OK != nvm3_user_get_mode(&config->mode)) {
    nvm3_deleteObject(NVM3_DEFAULT_HANDLE, MODE_KEY);
    config->mode = MODE_DEFAULT;
    nvm3_user_set_mode(config->mode);
  }

  // Initialize the setpoint config.
  if (SL_STATUS_OK != nvm3_user_get_setpoint(&config->setpoint)) {
    nvm3_deleteObject(NVM3_DEFAULT_HANDLE, SETPOINT_KEY);
    config->setpoint = SETPOINT_DEFAULT;
    nvm3_user_set_setpoint(config->setpoint);
  }

  // Initialize the hysteresis config.
  if (SL_STATUS_OK != nvm3_user_get_hysteresis(&config->hysteresis)) {
    nvm3_deleteObject(NVM3_DEFAULT_HANDLE, HYSTERESIS_KEY);
    config->hysteresis = HYSTERESIS_DEFAULT;
    nvm3_user_set_hysteresis(config->hysteresis);
  }

  // Initialize the lower threshold config.
  if (SL_STATUS_OK != nvm3_user_get_lower_threshold(&config->lower_threshold)) {
    nvm3_deleteObject(NVM3_DEFAULT_HANDLE, LOWER_THRESHOLD_KEY);
    config->lower_threshold = LOWER_THRESHOLD_DEFAULT;
    nvm3_user_set_lower_threshold(config->lower_threshold);
  }

  // Initialize the upper threshold config.
  if (SL_STATUS_OK != nvm3_user_get_upper_threshold(&config->upper_threshold)) {
    nvm3_deleteObject(NVM3_DEFAULT_HANDLE, UPPER_THRESHOLD_KEY);
    config->upper_threshold = UPPER_THRESHOLD_DEFAULT;
    nvm3_user_set_upper_threshold(config->upper_threshold);
  }

  // Initialize the notification enable config.
  if (SL_STATUS_OK != nvm3_user_get_alarm_enabled(&config->alarm_enabled)) {
    nvm3_deleteObject(NVM3_DEFAULT_HANDLE, ALARM_ENABLED_KEY);
    config->alarm_enabled = ALARM_ENABLED_DEFAULT;
    nvm3_user_set_alarm_enabled(config->alarm_enabled);
  }

  // Initialize the measurement interval config.
  if (SL_STATUS_OK
      != nvm3_user_get_measurement_interval(&config->measurement_interval)) {
    nvm3_deleteObject(NVM3_DEFAULT_HANDLE, MEASUREMENT_INTERVAL_KEY);
    config->measurement_interval = MEASUREMENT_INTERVAL_DEFAULT;
    nvm3_user_set_measurement_interval(config->measurement_interval);
  }

  // Initialize the buzzer volume config.
  if (SL_STATUS_OK != nvm3_user_get_buzzer_volume(&config->buzzer_volume)) {
    nvm3_deleteObject(NVM3_DEFAULT_HANDLE, BUZZER_VOLUME_KEY);
    config->buzzer_volume = BUZZER_VOLUME_DEFAULT;
    nvm3_user_set_buzzer_volume(config->buzzer_volume);
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Set NVM3 Mode
 ******************************************************************************/
sl_status_t nvm3_user_set_mode(uint8_t mode)
{
  if (mode > 1) {
    return SL_STATUS_INVALID_RANGE;
  }

  return nvm3_writeData(NVM3_DEFAULT_HANDLE,
                        MODE_KEY,
                        &mode,
                        sizeof(mode));
}

/***************************************************************************//**
 * Get NVM3 Mode
 ******************************************************************************/
sl_status_t nvm3_user_get_mode(uint8_t *mode)
{
  uint8_t value = 0;
  sl_status_t stt = nvm3_readData(NVM3_DEFAULT_HANDLE,
                                  MODE_KEY,
                                  &value,
                                  sizeof(value));
  if (stt != SL_STATUS_OK) {
    return stt;
  }

  if (value > 1) {
    return SL_STATUS_INVALID_RANGE;
  }

  *mode = value;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Set NVM3 Setpoint.
 ******************************************************************************/
Ecode_t nvm3_user_set_setpoint(int16_t setpoint)
{
  if ((setpoint > SETPOINT_MAX)
      || (setpoint < SETPOINT_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  return nvm3_writeData(NVM3_DEFAULT_HANDLE,
                        SETPOINT_KEY,
                        &setpoint,
                        sizeof(setpoint));
}

/***************************************************************************//**
 * Get NVM3 Setpoint.
 ******************************************************************************/
Ecode_t nvm3_user_get_setpoint(int16_t *setpoint)
{
  int16_t value = 0;
  sl_status_t stt = nvm3_readData(NVM3_DEFAULT_HANDLE,
                                  SETPOINT_KEY,
                                  &value,
                                  sizeof(value));
  if (stt != SL_STATUS_OK) {
    return stt;
  }

  if ((value > SETPOINT_MAX) || (value < SETPOINT_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  *setpoint = value;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Set NVM3 hysteresis.
 ******************************************************************************/
sl_status_t nvm3_user_set_hysteresis(int16_t hysteresis)
{
  if ((hysteresis > HYSTERESIS_MAX) || (hysteresis < HYSTERESIS_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  return nvm3_writeData(NVM3_DEFAULT_HANDLE,
                        HYSTERESIS_KEY,
                        &hysteresis,
                        sizeof(hysteresis));
}

/***************************************************************************//**
 * Get NVM3 hysteresis.
 ******************************************************************************/
sl_status_t nvm3_user_get_hysteresis(int16_t *hysteresis)
{
  int16_t value = 0;
  sl_status_t stt = nvm3_readData(NVM3_DEFAULT_HANDLE,
                                  HYSTERESIS_KEY,
                                  &value,
                                  sizeof(value));
  if (stt != SL_STATUS_OK) {
    return stt;
  }

  if ((value > HYSTERESIS_MAX) || (value < HYSTERESIS_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  *hysteresis = value;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Set NVM3 lower threshold.
 ******************************************************************************/
sl_status_t nvm3_user_set_lower_threshold(int16_t threshold)
{
  if ((threshold > LOWER_THRESHOLD_MAX)
      || (threshold < LOWER_THRESHOLD_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  return nvm3_writeData(NVM3_DEFAULT_HANDLE,
                        LOWER_THRESHOLD_KEY,
                        &threshold,
                        sizeof(threshold));
}

/***************************************************************************//**
 * Get Lower Threshold.
 ******************************************************************************/
sl_status_t nvm3_user_get_lower_threshold(int16_t *threshold)
{
  int16_t value = 0;
  sl_status_t stt = nvm3_readData(NVM3_DEFAULT_HANDLE,
                                  LOWER_THRESHOLD_KEY,
                                  &value,
                                  sizeof(value));
  if (stt != SL_STATUS_OK) {
    return stt;
  }

  if ((value > LOWER_THRESHOLD_MAX) || (value < LOWER_THRESHOLD_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  *threshold = value;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Set upper threshold
 ******************************************************************************/
sl_status_t nvm3_user_set_upper_threshold(int16_t threshold)
{
  if ((threshold > UPPER_THRESHOLD_MAX)
      || (threshold < UPPER_THRESHOLD_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  return nvm3_writeData(NVM3_DEFAULT_HANDLE,
                        UPPER_THRESHOLD_KEY,
                        &threshold,
                        sizeof(threshold));
}

/***************************************************************************//**
 * Get NVM3 upper threshold.
 ******************************************************************************/
sl_status_t nvm3_user_get_upper_threshold(int16_t *threshold)
{
  int16_t value = 0;
  sl_status_t stt = nvm3_readData(NVM3_DEFAULT_HANDLE,
                                  UPPER_THRESHOLD_KEY,
                                  &value,
                                  sizeof(value));
  if (stt != SL_STATUS_OK) {
    return stt;
  }

  if ((value > UPPER_THRESHOLD_MAX) || (value < UPPER_THRESHOLD_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  *threshold = value;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Set NVM3 alarm enabled
 ******************************************************************************/
sl_status_t nvm3_user_set_alarm_enabled(uint8_t enable)
{
  uint8_t data = enable ? 1:0;

  return nvm3_writeData(NVM3_DEFAULT_HANDLE,
                        ALARM_ENABLED_KEY,
                        (unsigned char *)&data,
                        sizeof(data));
}

/***************************************************************************//**
 * Get NVM3 alarm enabled
 ******************************************************************************/
sl_status_t nvm3_user_get_alarm_enabled(uint8_t *enable)
{
  uint8_t value = 0;
  sl_status_t stt = nvm3_readData(NVM3_DEFAULT_HANDLE,
                                  ALARM_ENABLED_KEY,
                                  &value,
                                  sizeof(value));
  if (stt != SL_STATUS_OK) {
    return stt;
  }

  if (value > 1) {
    return SL_STATUS_INVALID_RANGE;
  }

  *enable = value;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Set the Measurement Interval in seconds to NVM.
 ******************************************************************************/
sl_status_t nvm3_user_set_measurement_interval(uint8_t interval)
{
  if ((interval > MEASUREMENT_INTERVAL_MAX)
      || (interval < MEASUREMENT_INTERVAL_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  return nvm3_writeData(NVM3_DEFAULT_HANDLE,
                        MEASUREMENT_INTERVAL_KEY,
                        &interval,
                        sizeof(interval));
}

/***************************************************************************//**
 *  Get the Measurement Interval in seconds from NVM.
 ******************************************************************************/
sl_status_t nvm3_user_get_measurement_interval(uint8_t *interval)
{
  uint8_t value = 0;
  sl_status_t stt = nvm3_readData(NVM3_DEFAULT_HANDLE,
                                  MEASUREMENT_INTERVAL_KEY,
                                  &value,
                                  sizeof(value));
  if (stt != SL_STATUS_OK) {
    return stt;
  }

  if ((value > MEASUREMENT_INTERVAL_MAX)
      || (value < MEASUREMENT_INTERVAL_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  *interval = value;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Set the Buzzer Volume to NVM.
 ******************************************************************************/
sl_status_t nvm3_user_set_buzzer_volume(uint8_t volume)
{
  if ((volume > BUZZER_VOLUME_MAX) || (volume < BUZZER_VOLUME_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  return nvm3_writeData(NVM3_DEFAULT_HANDLE,
                        BUZZER_VOLUME_KEY,
                        &volume,
                        sizeof(volume));
}

/***************************************************************************//**
 *  Get the Buzzer Volume from NVM.
 ******************************************************************************/
sl_status_t nvm3_user_get_buzzer_volume(uint8_t *volume)
{
  uint8_t value = 0;
  sl_status_t stt = nvm3_readData(NVM3_DEFAULT_HANDLE,
                                  BUZZER_VOLUME_KEY,
                                  &value,
                                  sizeof(value));
  if (stt != SL_STATUS_OK) {
    return stt;
  }

  if ((value > BUZZER_VOLUME_MAX) || (value < BUZZER_VOLUME_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  *volume = value;
  return SL_STATUS_OK;
}
