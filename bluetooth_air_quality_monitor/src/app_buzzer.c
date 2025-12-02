/***************************************************************************//**
 * @file app_buzzer.c
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
#include "mikroe_cmt_8540s_smt.h"
#include "sl_pwm_instances.h"
#include "app_buzzer.h"

// -----------------------------------------------------------------------------
// Define
#define VOLUME 100 // goes up to 100

sl_status_t buzzer_init(void)
{
  sl_status_t sc;

  sc = mikroe_cmt_8540s_smt_init(&sl_pwm_mikroe);
  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  mikroe_cmt_8540s_smt_play_sound(MIKROE_BUZZ2_NOTE_A6, VOLUME, 0);
  sl_sleeptimer_delay_millisecond(100);

  return SL_STATUS_OK;
}

void buzzer_set_volume(uint8_t volume)
{
  mikroe_cmt_8540s_smt_set_duty_cycle((float)volume / 10.0);
}

void buzzer_control(bool on)
{
  if (on) {
    mikroe_cmt_8540s_smt_pwm_start();
  } else {
    mikroe_cmt_8540s_smt_pwm_stop();
  }
}
