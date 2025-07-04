/***************************************************************************//**
 * @file app_micro_oled.h
 * @brief Header file for micro OLED
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
#ifndef APP_MICRO_OLED_H_
#define APP_MICRO_OLED_H_

#include <stdio.h>
#include "app_ph2.h"

/*******************************************************************************
 * @brief
 *  Initialize the micro OLED.
 ******************************************************************************/
void app_sparkfun_micro_oled_init(void);

/*******************************************************************************
 * @brief
 *  Display the pH value on the screen.
 *
 * @param[in] pH_display_value   Value to be displayed
 ******************************************************************************/
void app_sparkfun_micro_oled_process(float pH_display_value);

#endif /* APP_MICRO_OLED_H_ */
