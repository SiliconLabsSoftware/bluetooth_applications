/***************************************************************************//**
 * @file app_thermostat_buzzer.c
 * @brief Buzzer application code
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
#include "app_actuator.h"
#include "sl_simple_led_instances.h"

// -----------------------------------------------------------------------------
// Define
#define VOLUME 100 // goes up to 100

sl_status_t actuator_init(void)
{
  return SL_STATUS_OK;
}

void actuator_control(bool on)
{
  if (on) {
    sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(0));
  } else {
    sl_led_turn_off(SL_SIMPLE_LED_INSTANCE(0));
  }
}
