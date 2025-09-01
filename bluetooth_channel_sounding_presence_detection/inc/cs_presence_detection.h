/***************************************************************************//**
 * @file cs_presence_detection.h
 * @brief CS Based Presence Detection API
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

#ifndef CS_PRESENCE_DETECTION_H_
#define CS_PRESENCE_DETECTION_H_

#include "app_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
  CS_PRESENCE_DETECTED     = 0,
  CS_PRESENCE_NOT_DETECTED = 1,
  CS_PRESENCE_MAX          = 2,
} cs_presence_detection_t;

// -----------------------------------------------------------------------------
// Function declarations

/*******************************************************************************
 * cs_presence_detection_init.
 ******************************************************************************/
sl_status_t cs_presence_detection_init(void);

/*******************************************************************************
 * cs_presence_detection_processing
 * Processing distance value to detect presence.
 ******************************************************************************/
sl_status_t cs_presence_detection_processing(float measured_distance);

#ifdef __cplusplus
}
#endif

/** @} (end addtogroup cs_based_pc_locking) */
#endif /* CS_PRESENCE_DETECTION_H_ */
