/***************************************************************************//**
 * @file app_config.h
 * @brief CS Initiator Display example configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
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
 ******************************************************************************/

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// Measurement unit
#define MEASUREMENT_UNIT_METERS   (0U)
#define MEASUREMENT_UNIT_FEET     (1U)

// Channel sounding measurement unit
#define CS_MEASUREMENT_UNIT           MEASUREMENT_UNIT_METERS

// Maximum measurable value of channel sounding with CS_MEASUREMENT_UNIT
#define MAX_MEASURABLE_VALUE_OF_CS    (30)

// <<< Use Configuration Wizard in Context Menu >>>

// <o SYSTEM_MIN_TX_POWER_DBM> Connection minimum TX Power <-127..20>
// <i> Connection minimum TX Power in dBm
// <i> Default: -3
#define SYSTEM_MIN_TX_POWER_DBM               -3

// <o SYSTEM_MAX_TX_POWER_DBM> Connection maximum TX Power <-127..20>
// <i> Connection minimum TX Power in dBm. Must be greater than the minimum value.
// <i> Default: 20
#define SYSTEM_MAX_TX_POWER_DBM               20

// <q ALWAYS_INIT_TRACE> Enable trace on init
// <i> If enabled, this option bypasses trace initialization with push button.
// <i> Default: 0
#define ALWAYS_INIT_TRACE    0

// <<< end of configuration section >>>


/***********************************************************************************************//**
 * @addtogroup cs_initiator_display
 * @{
 **************************************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <q CS_INITIATOR_DISPLAY_LOG> Enable initiator display log
// <i> Default: 1
// <i> Enable Initiator event buffer managers own log
#ifndef CS_INITIATOR_DISPLAY_LOG
#define CS_INITIATOR_DISPLAY_LOG              (1)
#endif

// <<< end of configuration section >>>

#endif // APP_CONFIG_H
