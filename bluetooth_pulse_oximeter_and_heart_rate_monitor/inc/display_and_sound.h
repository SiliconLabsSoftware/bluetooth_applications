/***************************************************************************//**
 * @file display_and_sound.h
 * @brief pulse_oximeter_and_heart_rate_monitor display and sound interface
 * @version 1.0
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
#ifndef _DISPLAY_AND_SOUND_H_
#define _DISPLAY_AND_SOUND_H_
#include "sl_component_catalog.h"
#include "sl_status.h"
#include "pulse_oximeter_and_heart_rate_monitor.h"

sl_status_t display_and_sound_init(void);
sl_status_t display_and_sound_process(const pom_and_hr_monitor_data_t *data,
                                      const user_config_t *config);

#endif // _DISPLAY_AND_SOUND_H_
