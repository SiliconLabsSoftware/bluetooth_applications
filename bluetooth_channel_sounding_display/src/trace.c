/***************************************************************************//**
 * @file trace.c
 * @brief Debug trace.
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
 *******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/

#include "sl_component_catalog.h"
#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT) \
  && defined(SL_CATALOG_BGAPI_TRACE_PRESENT)
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

#include <stdbool.h>
#include "sli_bgapi_trace.h"
#include "app_config.h"
#include "rtl_log.h"

#include "app_log.h"
#include "iostream_bgapi_trace.h"

#ifdef SL_CATALOG_SIMPLE_LED_PRESENT
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"
#define led_on()  sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(0))
#define led_off() sl_led_turn_off(SL_SIMPLE_LED_INSTANCE(0))
#else
#define led_on()
#define led_off()
#endif // SL_CATALOG_SIMPLE_LED_PRESENT

static bool trace_enabled = false;
static bool button_pressed = false;

static void enable_trace(void);
static void disable_trace(void);

void sl_button_on_change(const sl_button_t *handle)
{
  if (handle != SL_SIMPLE_BUTTON_INSTANCE(0)) {
    return;
  }
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    button_pressed = true;
  }
}

void trace_init(void)
{
  app_log_iostream_set(iostream_bgapi_trace_handle);
#if ALWAYS_INIT_TRACE == 0
  if (sl_button_get_state(SL_SIMPLE_BUTTON_INSTANCE(0))
      != SL_SIMPLE_BUTTON_PRESSED) {
    return;
  }
#endif
  enable_trace();
  sli_bgapi_trace_sync();
}

void trace_step(void)
{
  if (!button_pressed) {
    return;
  }
  button_pressed = false;
  if (trace_enabled) {
    disable_trace();
  } else {
    enable_trace();
  }
}

static void enable_trace(void)
{
  sli_bgapi_trace_start();
  rtl_log_init();
  led_on();
  trace_enabled = true;
}

static void disable_trace(void)
{
  trace_enabled = false;
  led_off();
  rtl_log_deinit();
  sli_bgapi_trace_stop();
}

#else // SL_CATALOG_SIMPLE_BUTTON_PRESENT && SL_CATALOG_BGAPI_TRACE_PRESENT
void trace_init(void)
{
  // Trace is inactive
}

void trace_step(void)
{
  // Trace is inactive
}

#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT && SL_CATALOG_BGAPI_TRACE_PRESENT
