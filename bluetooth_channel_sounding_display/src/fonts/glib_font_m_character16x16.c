/***************************************************************************//**
 * @file
 * @brief Silicon Labs Graphics Library: GLIB font containing 'm' character
 * 16x16 pixels
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

/* Standard C header files */
#include <stdint.h>
#include "glib.h"

/** @brief Pixel data for the "GLIB_Font16x16" font. */
static const uint16_t GLIB_Font16x16PixMap[] =
{
  0x0000, 0x0000, 0x3c7b, 0x7eff, 0x7fff, 0x73cf, 0x71c7, 0x71c7, 0x71c7,
  0x71c7, 0x71c7, 0x71c7, 0x71c7, 0x71c7, 0x71c7, 0x71c7,
};

/**
 * @brief 16x16 pixels font containing 'm' character.
 */
const GLIB_Font_t GLIB_Font16x16 = { (void *)GLIB_Font16x16PixMap,
                                     sizeof(GLIB_Font16x16PixMap),
                                     sizeof(GLIB_Font16x16PixMap[0]),
                                     1, 16, 16, 0, 0, NumbersOnlyFont };
