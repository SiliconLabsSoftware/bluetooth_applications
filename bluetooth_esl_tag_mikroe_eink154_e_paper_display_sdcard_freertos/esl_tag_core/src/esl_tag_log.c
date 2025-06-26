/***************************************************************************//**
 * @file
 * @brief ESL Tag core logging weak implementation for keeping the code size low
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_common.h"
#include "esl_tag_log.h"
#include "esl_log_config.h"
#include "sl_iostream.h"
#include "app_log.h"

const char *sli_bt_esl_log_get_ordinal(uint32_t value)
{
  // normalize the range of the value
  value %= 100;

  if ((value > 3) && (value < 21)) {
    value = 4;  // shortcut for exceptions
  }

  switch (value % 10) {
    case 1:
      return "st";

    case 2:
      return "nd";

    case 3:
      return "rd";

    default:
      return "th";
  }
}

// Check if given component has logging enabled
static bool component_log_enabled(uint8_t component)
{
  bool enabled = (ESL_CORE_LOG_ENABLE != 0);

  // is enabled by config?
  if (enabled) {
    switch (component & ESL_LOG_FLAGS_MASK) {
      case ESL_LOG_COMPONENT_CORE:
        if (ESL_LOG_COMPONENT_CORE_ENABLE == 0) {
          enabled = false;
        }
        break;

      case ESL_LOG_COMPONENT_APP:
        if (ESL_LOG_COMPONENT_APP_ENABLE == 0) {
          enabled = false;
        }
        break;

      case ESL_LOG_COMPONENT_DISPLAY:
        if (ESL_LOG_COMPONENT_DISPLAY_ENABLE == 0) {
          enabled = false;
        }
        break;

      case ESL_LOG_COMPONENT_RAM_IMAGE:
        if (ESL_LOG_COMPONENT_RAM_IMAGE_ENABLE == 0) {
          enabled = false;
        }
        break;

      case ESL_LOG_COMPONENT_NVM_IMAGE:
        if (ESL_LOG_COMPONENT_NVM_IMAGE_ENABLE == 0) {
          enabled = false;
        }
        break;

      case ESL_LOG_COMPONENT_LED:
        if (ESL_LOG_COMPONENT_LED_ENABLE == 0) {
          enabled = false;
        }
        break;

      case ESL_LOG_COMPONENT_OTS:
        if (ESL_LOG_COMPONENT_OTS_ENABLE == 0) {
          enabled = false;
        }
        break;

      case ESL_LOG_COMPONENT_SENSOR:
        if (ESL_LOG_COMPONENT_SENSOR_ENABLE == 0) {
          enabled = false;
        }
        break;

      default:
        // do not block any future components by default
        break;
    }
  }

  return enabled;
}

void sli_bt_esl_logger(uint8_t    component,
                       uint8_t    level,
                       const char *fmt,
                       ...)
{
#if defined(APP_LOG_ENABLE) && APP_LOG_ENABLE
  if (component_log_enabled(component)) {
    va_list va;
    va_start(va, fmt);

    if (app_log_check_level(level)) {
      if (!(component & ESL_LOG_FLAG_APPEND)) {
        app_log_nl();
      }

      _app_log_print_color(level);

      if (!(component & (ESL_LOG_FLAG_NOHEADER | ESL_LOG_FLAG_APPEND))) {
        _app_log_print_prefix(level);
        _app_log_time();
        _app_log_counter();
        _app_log_print_trace();
      }

      sl_iostream_vprintf(app_log_iostream, fmt, va);
    }
    va_end(va);
  }
#else // defined(APP_LOG_ENABLE) && APP_LOG_ENABLE
  (void)component;
  (void)level;
  (void)fmt;
#endif // defined(APP_LOG_ENABLE) && APP_LOG_ENABLE
}

void sli_bt_esl_log_hex_dump(uint8_t component,
                             uint8_t level,
                             void    *p_data,
                             uint8_t len)
{
#if defined(APP_LOG_ENABLE) && APP_LOG_ENABLE
  if (component_log_enabled(component)) {
    if (app_log_check_level(level)) {
      uint8_t *tmp = (uint8_t *)p_data;

      if (!(component & ESL_LOG_FLAG_APPEND)) {
        app_log_nl();
      }

      _app_log_print_color(level);

      if (!(component & (ESL_LOG_FLAG_NOHEADER | ESL_LOG_FLAG_APPEND))) {
        _app_log_print_prefix(level);
        _app_log_time();
        _app_log_counter();
        _app_log_print_trace();
      }

      while (len--) {
        if (tmp != p_data) {
          app_log_append(APP_LOG_HEXDUMP_SEPARATOR);
        }

        app_log_append(APP_LOG_HEXDUMP_PREFIX);
        app_log_append(APP_LOG_HEXDUMP_FORMAT,
                       *tmp++);
      }
    }
  }
#else // defined(APP_LOG_ENABLE) && APP_LOG_ENABLE
  (void)component;
  (void)level;
  (void)p_data;
  (void)len;
#endif // defined(APP_LOG_ENABLE) && APP_LOG_ENABLE
}

void sli_bt_esl_assert_func(const char *func,
                            const char *file,
                            const char *expr,
                            int        line)
{
  sli_bt_esl_logger(ESL_LOG_COMPONENT_CLI,
                    ESL_LOG_LEVEL_CRITICAL,
                    "Assert failed: (%s) in file: %s, function: %s @ line: %d.",
                    expr, file, func, line);
  while (1) {  // Halt
  }
}
