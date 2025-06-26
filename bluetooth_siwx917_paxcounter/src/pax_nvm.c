/***************************************************************************//**
 * @file pax_nvm.c
 * @brief pax counter nvm source file.
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

#include "pax_nvm.h"

uint32_t rokkit_hash(const char *data, uint16_t len)
{
  // This is mostly Paul Hsieh's original code
  uint32_t hash, tmp;
  int rem;

  if ((len <= 0) || (data == 0)) {
    return 0;
  }

  hash = len;
  rem = len & 3;
  len >>= 2;

  /* Main loop */
  while (len > 0) {
    hash += *((uint16_t *) data);

    /* To make a long story short, the C standard states that the
     * shift operator's operands must be promoted to (unsigned) int,
     * which is (usually) 32 bits wide on PC and 16 on Arduino. This
     * results in different behaviour, since part of the result gets
     * truncated on Arduino, so we cast the result to make sure all
     * bits are kept.
     */
    tmp = ((uint32_t) (*((uint16_t *) (data + 2))) << 11) ^ hash;

    hash = (hash << 16) ^ tmp;
    data += 2 * sizeof (uint16_t);
    hash += hash >> 11;
    len--;
  }

  /* Handle end cases */
  switch (rem) {
    case 3:
      hash += *((uint16_t *) data);
      hash ^= hash << 16;
      hash ^= ((signed char) data[2]) << 18;
      hash += hash >> 11;
      break;

    case 2:
      hash += *((uint16_t *) data);
      hash ^= hash << 11;
      hash += hash >> 17;
      break;

    case 1:
      hash += (signed char) *data;
      hash ^= hash << 10;
      hash += hash >> 1;
  }

  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;

  return hash;
}

void initialise_counters(void)
{
  uint32_t type;
  size_t len;
  Ecode_t err;

  // check if the designated keys contain counters, and initialise if needed.
  err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, &type, &len);
  if ((err != ECODE_NVM3_OK) || (type != NVM3_OBJECTTYPE_COUNTER)) {
    nvm3_writeCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, 0);
  }

  err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, &type,
                           &len);
  if ((err != ECODE_NVM3_OK) || (type != NVM3_OBJECTTYPE_COUNTER)) {
    nvm3_writeCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, 0);
  }
}

sl_status_t nvm3_read(nvm3_ObjectKey_t key,
                      uint8_t bd_ascii_addr[BD_ADDR_LEN],
                      uint32_t *last_seen)
{
  if (key > MAX_DATA_KEY) {
    return ECODE_NVM3_ERR_KEY_INVALID;
  }

  uint32_t type;
  size_t len;
  device_id_t stored_data;

  Ecode_t err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, key, &type, &len);
  if ((err != ECODE_NVM3_OK) || (type != NVM3_OBJECTTYPE_DATA)) {
    return ECODE_NVM3_ERR_READ_FAILED;
  }

  err = nvm3_readData(NVM3_DEFAULT_HANDLE,
                      key,
                      &stored_data,
                      sizeof(stored_data));
  if (err == ECODE_NVM3_OK) {
    memcpy(bd_ascii_addr, stored_data.bd_ascii_addr, BD_ADDR_LEN);
    *last_seen = stored_data.last_seen;
  }

  return err;
}

sl_status_t nvm3_add_device(uint32_t key, device_id_t *data, uint32_t len)
{
  if (key > MAX_DATA_KEY) {
    return ECODE_NVM3_ERR_KEY_INVALID;
  }

  return nvm3_writeData(NVM3_DEFAULT_HANDLE, key, (uint8_t *)data, len);
}

sl_status_t nvm3_app_delete(uint32_t key)
{
  if (key > MAX_DATA_KEY) {
    return ECODE_NVM3_ERR_KEY_INVALID;
  }

  Ecode_t err = nvm3_deleteObject(NVM3_DEFAULT_HANDLE, key);
  if (err == ECODE_NVM3_OK) {
    nvm3_incrementCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, NULL);
  }

  return err;
}

void nvm3_update(uint32_t *pax_count)
{
  nvm3_ObjectKey_t keys[MAX_OBJECT_COUNT];
  size_t objects_count = nvm3_enumObjects(NVM3_DEFAULT_HANDLE,
                                          (uint32_t *)keys,
                                          MAX_OBJECT_COUNT,
                                          MIN_DATA_KEY,
                                          MAX_DATA_KEY);
  device_id_t stored_data;
  uint32_t type;
  size_t len;

  for (size_t i = 0; i < objects_count; i++) {
    nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, keys[i], &type, &len);

    if (type == NVM3_OBJECTTYPE_DATA) {
      if (nvm3_readData(NVM3_DEFAULT_HANDLE, keys[i], &stored_data,
                        len) == ECODE_NVM3_OK) {
        uint32_t age = osKernelGetTickCount() - stored_data.last_seen;
        if (age > OCCUPANCY_TIMEOUT_MS) {
          nvm3_app_delete(keys[i]);
        }
      }
    }
  }

  uint32_t deletes = 0, writes = 0;
  if (nvm3_readCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY,
                       &deletes) != ECODE_NVM3_OK) {
    deletes = 0;
  }
  if (nvm3_readCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY,
                       &writes) != ECODE_NVM3_OK) {
    writes = 0;
  }

  *pax_count = (writes > deletes) ? (writes - deletes) : 0;
}
