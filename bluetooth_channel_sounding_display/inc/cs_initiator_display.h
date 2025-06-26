/***************************************************************************//**
 * @file cs_initiator_display.h
 * @brief CS Initiator display API
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

#ifndef CS_INITIATOR_DISPLAY_H
#define CS_INITIATOR_DISPLAY_H

/*******************************************************************************
 * @addtogroup cs_initiator_display
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Includes

#include "sl_bt_api.h"
#include "sl_status.h"
#include "app_log.h"
#include "sl_component_catalog.h"
#include "glib.h"
#include "dmd.h"
#include "app_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
// Macros

// Component logging
#if defined(SL_CATALOG_APP_LOG_PRESENT) && CS_INITIATOR_DISPLAY_LOG
#define LOG_PREFIX                "[lcd] "
#define NL                        APP_LOG_NL
#define display_log_debug(...)    app_log_debug(LOG_PREFIX  __VA_ARGS__)
#define display_log_info(...)     app_log_info(LOG_PREFIX  __VA_ARGS__)
#define display_log_warning(...)  app_log_warning(LOG_PREFIX  __VA_ARGS__)
#define display_log_error(...)    app_log_error(LOG_PREFIX  __VA_ARGS__)
#define display_log_critical(...) app_log_critical(LOG_PREFIX  __VA_ARGS__)
#else
#define NL
#define display_log_debug(...)
#define display_log_info(...)
#define display_log_warning(...)
#define display_log_error(...)
#define display_log_critical(...)
#endif // defined(SL_CATALOG_APP_LOG_PRESENT) && CS_INITIATOR_DISPLAY_LOG

typedef struct {
  float distance;
} cs_initiator_display_content_t;

// -----------------------------------------------------------------------------
// Function declarations

/*******************************************************************************
 * Initialize the display.
 ******************************************************************************/
sl_status_t cs_initiator_display_init(void);

/*******************************************************************************
 * CS initiator display measured distance.
 ******************************************************************************/
void cs_initiator_display_distance_measurement(float measured_distance);

/*******************************************************************************
 * CS initiator display distance progress bar
 ******************************************************************************/
void cs_initiator_display_progress_bar(float measured_distance);

/*******************************************************************************
 * Update the display.
 ******************************************************************************/
void cs_initiator_display_update(void);

#ifdef __cplusplus
}
#endif

/** @} (end addtogroup cs_initiator_display) */
#endif // CS_INITIATOR_DISPLAY_H
