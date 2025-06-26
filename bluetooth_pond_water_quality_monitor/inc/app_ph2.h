/***************************************************************************//**
 * @file app_ph2.h
 * @brief Header file for pH2 Click
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 ********************************************************************************
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
 * This code has been minimally tested to ensure that it builds and is suitable
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/
#ifndef APP_PH2_H_
#define APP_PH2_H_

#include "app_assert.h"

#define READING_INTERVAL_MSEC        (1000)
#define NEUTRAL_PH_VALUE             (7)
#define CALIBRATED_NVM_ADDRESS       (0x4001)
#define MAX_PH_VALUE                 (14)

/*******************************************************************************
 * @brief
 *  Initialize the pH2 Click.
 ******************************************************************************/
void app_mikroe_ph2_init(void);

/*******************************************************************************
 * @brief
 *  Calibrate the pH2 Click.
 ******************************************************************************/
void app_mikroe_ph2_calibrate(void);

/*******************************************************************************
 * @brief
 *  Get the pH2 value
 *
 * @param[in] pH_value     Pointer to the pH value to be changed
 ******************************************************************************/
void app_mikroe_ph2_process(float *pH_value);

#endif /* APP_PH2_H_ */
