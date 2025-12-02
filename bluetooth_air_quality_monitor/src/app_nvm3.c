/***************************************************************************//**
 * @file app_nvm3.c
 * @brief NVM3 functions
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
#include <string.h>

#include "nvm3_default_config.h"
#include "app_nvm3.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Max and min keys for data objects
#define ALARM_ENABLED_KEY                   (NVM3_KEY_MIN)
#define MEASUREMENT_INTERVAL_KEY            (NVM3_KEY_MIN + 1)
#define BUZZER_VOLUME_KEY                   (NVM3_KEY_MIN + 2)
#define CO2_THRESHOLD_PPM_KEY               (NVM3_KEY_MIN + 3)
#define TVOC_THRESHOLD_PPB_KEY              (NVM3_KEY_MIN + 4)

// Use the default nvm3 handle from nvm3_default.h
#define NVM3_DEFAULT_HANDLE                 nvm3_defaultHandle

#define ALARM_ENABLED_MIN                   0
#define ALARM_ENABLED_MAX                   1
#define ALARM_ENABLED_DEFAULT               1

#define MEASUREMENT_INTERVAL_MIN            1
#define MEASUREMENT_INTERVAL_MAX            30
#define MEASUREMENT_INTERVAL_DEFAULT        10

#define BUZZER_VOLUME_MIN                   1
#define BUZZER_VOLUME_MAX                   10
#define BUZZER_VOLUME_DEFAULT               6

#define CO2_THRESHOLD_PPM_MIN               400
#define CO2_THRESHOLD_PPM_MAX               8192
#define CO2_THRESHOLD_PPM_DEFAULT           1000

#define TVOC_THRESHOLD_PPB_MIN              1
#define TVOC_THRESHOLD_PPB_MAX              1187
#define TVOC_THRESHOLD_PPB_DEFAULT          100

/***************************************************************************//**
 * Initialize NVM3 example.
 ******************************************************************************/
sl_status_t nvm3_user_init(air_quality_config_t *config)
{
  sl_status_t stt;

  // This will call nvm3_open() with default parameters for
  // memory base address and size, cache size, etc.
  stt = nvm3_initDefault();
  if (stt != SL_STATUS_OK) {
    return stt;
  }

  // Initialise the alarm enable config.
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

  // Initialise the co2 threshold config.
  if (SL_STATUS_OK != nvm3_user_get_co2_threshold(&config->co2_threshold)) {
    nvm3_deleteObject(NVM3_DEFAULT_HANDLE, CO2_THRESHOLD_PPM_KEY);
    config->co2_threshold = CO2_THRESHOLD_PPM_DEFAULT;
    nvm3_user_set_co2_threshold(config->co2_threshold);
  }

  // Initialise the tvoc threshold config.
  if (SL_STATUS_OK != nvm3_user_get_co2_threshold(&config->tvoc_threshold)) {
    nvm3_deleteObject(NVM3_DEFAULT_HANDLE, TVOC_THRESHOLD_PPB_KEY);
    config->tvoc_threshold = TVOC_THRESHOLD_PPB_DEFAULT;
    nvm3_user_set_tvoc_threshold(config->tvoc_threshold);
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Set the alarm enabled to NVM.
 ******************************************************************************/
sl_status_t nvm3_user_set_alarm_enabled(uint8_t enable)
{
  uint8_t data = enable ? 1 : 0;
  sl_status_t stt;

  stt = nvm3_writeData(NVM3_DEFAULT_HANDLE,
                       ALARM_ENABLED_KEY,
                       (unsigned char *)&data,
                       sizeof(data));
  return stt;
}

/***************************************************************************//**
 *  Get the alarm enabled from NVM.
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
 *  Set the Measurement interval in seconds value to NVM.
 ******************************************************************************/
sl_status_t nvm3_user_set_measurement_interval(uint8_t interval)
{
  sl_status_t stt;

  if ((interval > MEASUREMENT_INTERVAL_MAX)
      || (interval < MEASUREMENT_INTERVAL_MIN)) {
    return ECODE_NVM3_ERR_PARAMETER;
  }
  stt = nvm3_writeData(NVM3_DEFAULT_HANDLE,
                       MEASUREMENT_INTERVAL_KEY,
                       (unsigned char *)&interval,
                       sizeof(interval));
  return stt;
}

/***************************************************************************//**
 *  Get the Measurement interval in seconds value from NVM.
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
 *  Set the buzzer volume to NVM.
 ******************************************************************************/
sl_status_t nvm3_user_set_buzzer_volume(uint8_t volume)
{
  sl_status_t stt;

  if ((volume > BUZZER_VOLUME_MAX) || (volume < BUZZER_VOLUME_MIN)) {
    return ECODE_NVM3_ERR_PARAMETER;
  }
  stt = nvm3_writeData(NVM3_DEFAULT_HANDLE,
                       BUZZER_VOLUME_KEY,
                       (unsigned char *)&volume,
                       sizeof(volume));
  return stt;
}

/***************************************************************************//**
 *  Get the buzzer volume from NVM.
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

/***************************************************************************//**
 *  Set the alarm threshold for CO2 level in ppm to NVM.
 ******************************************************************************/
sl_status_t nvm3_user_set_co2_threshold(uint16_t threshold)
{
  sl_status_t stt;

  if ((threshold > CO2_THRESHOLD_PPM_MAX)
      || (threshold < CO2_THRESHOLD_PPM_MIN)) {
    return ECODE_NVM3_ERR_PARAMETER;
  }

  stt = nvm3_writeData(NVM3_DEFAULT_HANDLE,
                       CO2_THRESHOLD_PPM_KEY,
                       (unsigned char *)&threshold,
                       sizeof(threshold));
  return stt;
}

/***************************************************************************//**
 *  Get the alarm threshold for CO2 level in ppm from NVM.
 ******************************************************************************/
sl_status_t nvm3_user_get_co2_threshold(uint16_t *threshold)
{
  uint16_t value = 0;
  sl_status_t stt = nvm3_readData(NVM3_DEFAULT_HANDLE,
                                  CO2_THRESHOLD_PPM_KEY,
                                  &value,
                                  sizeof(value));
  if (stt != SL_STATUS_OK) {
    return stt;
  }

  if ((value > CO2_THRESHOLD_PPM_MAX) || (value < CO2_THRESHOLD_PPM_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  *threshold = value;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Set the alarm threshold for tVOC level in ppb to NVM.
 ******************************************************************************/
sl_status_t nvm3_user_set_tvoc_threshold(uint16_t threshold)
{
  sl_status_t stt;

  if ((threshold > TVOC_THRESHOLD_PPB_MAX)
      || (threshold < TVOC_THRESHOLD_PPB_MIN)) {
    return ECODE_NVM3_ERR_PARAMETER;
  }
  stt = nvm3_writeData(NVM3_DEFAULT_HANDLE,
                       TVOC_THRESHOLD_PPB_KEY,
                       (unsigned char *)&threshold,
                       sizeof(threshold));
  return stt;
}

/***************************************************************************//**
 *  Get the alarm threshold for tVOC level in ppb from NVM.
 ******************************************************************************/
sl_status_t nvm3_user_get_tvoc_threshold(uint16_t *threshold)
{
  uint16_t value = 0;
  sl_status_t stt = nvm3_readData(NVM3_DEFAULT_HANDLE,
                                  TVOC_THRESHOLD_PPB_KEY,
                                  &value,
                                  sizeof(value));
  if (stt != SL_STATUS_OK) {
    return stt;
  }

  if ((value > TVOC_THRESHOLD_PPB_MAX) || (value < TVOC_THRESHOLD_PPB_MIN)) {
    return SL_STATUS_INVALID_RANGE;
  }

  *threshold = value;
  return SL_STATUS_OK;
}
