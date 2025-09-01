/***************************************************************************//**
 * @file cs_based_pc_locking.h
 * @brief CS Based PC Locking API
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

#ifndef CS_BASED_PC_LOCKING_H
#define CS_BASED_PC_LOCKING_H

/*******************************************************************************
 * @addtogroup cs_based_pc_locking
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Includes
#include "sl_component_catalog.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum cs_presece_detect_e
{
  CS_PRESENCE_DETECTED = 0,
  CS_PRESENCE_NOT_DETECTED = 1,
  CS_PRESENCE_MAX
}cs_presece_detect_t;

typedef enum cs_pc_locking_state_e
{
  CS_PC_STATE_UNLOCK = 0,
  CS_PC_STATE_LOCKED = 1,
  CS_PC_STATE_MAX
}cs_pc_locking_state_t;

// -----------------------------------------------------------------------------
// Function declarations

/*******************************************************************************
 * cs_based_pc_locking_init.
 ******************************************************************************/
sl_status_t cs_based_pc_locking_init(void);

/*******************************************************************************
 * cs_based_pc_locking_processing
 * Processing distance value to detect presence and trigger lock the paired PC.
 ******************************************************************************/
sl_status_t cs_based_pc_locking_processing(float measured_distance,
                                           bool notification_enabled);

#ifdef __cplusplus
}
#endif

/** @} (end addtogroup cs_based_pc_locking) */
#endif // CS_BASED_PC_LOCKING_H
