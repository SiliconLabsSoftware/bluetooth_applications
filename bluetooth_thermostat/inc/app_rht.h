/***************************************************************************//**
 * @file app_rht.h
 * @brief Measure temperature and humidity with SHTC3 sensor
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

#ifndef APP_RHT_H
#define APP_RHT_H

#include "sl_status.h"
#include "mikroe_shtc3.h"

/***************************************************************************//**
 * @brief
 *    Typedef for specifying the packed measurement data.
 ******************************************************************************/
typedef mikroe_shtc3_measurement_data_t app_rht_data_t;

/***************************************************************************//**
 * @brief
 *    Initialize RHT SHTC3 sensor.
 *
 ******************************************************************************/
sl_status_t rht_init(void);

sl_status_t rht_get_data(app_rht_data_t *data);

#endif // APP_RHT_H
