/***************************************************************************//**
 * @file
 * @brief ESL Tag SD card Image logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "esl_tag_image_config.h"
#include "esl_tag_ots_server.h"
#include "esl_tag_image_core.h"
#include "esl_tag_errors.h"
#include "esl_tag_image.h"
#include "esl_tag_core.h"
#include "esl_tag_log.h"
#include "sl_common.h"
#include "gatt_db.h"
#include "ff.h"

#define IMAGE_ROOT_DIR    ESL_TAG_SDCARD_IMAGE_ROOT_DIR
#define IMAGE_NAME_PREFIX ESL_TAG_SDCARD_IMAGE_NAME_PREFIX

// Single image object storage type
typedef PACKSTRUCT (struct {
  uint16_t  max_size;
  uint16_t  size;
  uint32_t  image_index;
}) esl_image_object_t;

// Internal image registry type
typedef PACKSTRUCT (struct {
  esl_image_object_t  *active_image;
  uint8_t             *next_object;
  uint8_t             images_count;
}) esl_image_registry_t;

static uint8_t chunk_data[ESL_TAG_SDCARD_IMAGE_CHUNK_BUFFER_SIZE];

// Image registry structure
static esl_image_registry_t image_registry = { 0 };

// Image object array - max. number of elements is defined in config header
static esl_image_object_t   image_object[ESL_TAG_MAX_IMAGES] = { 0 };

// Image pool - the static storage for all image data
// static uint8_t              image_pool[ESL_TAG_RAM_IMAGE_POOL_SIZE];

void esl_image_receive_finished(void)
{
}

sl_status_t esl_image_chunk_received(uint8_t const *data,
                                     uint32_t offset,
                                     uint16_t length)
{
  sl_status_t result = SL_STATUS_INVALID_INDEX;
  esl_image_object_t *active_image = image_registry.active_image;

  // ESL NVM image: data must fit to NVM object. Please check NVM config!
  sl_bt_esl_assert(length <= ESL_TAG_SDCARD_IMAGE_CHUNK_BUFFER_SIZE);

  // if there's a valid image object selected
  if (active_image != NULL) {
    size_t decompressed_size = 0;

    // check if it is the start from the beginning
    if (offset == 0) {
      DIR dir;

      if (FR_OK == f_opendir(&dir, IMAGE_ROOT_DIR)) {
        f_closedir(&dir);
      } else {
        f_mkdir(IMAGE_ROOT_DIR);
      }

      // (re)initialize decompressor on start
      esl_image_unpack_init();
      sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                    ESL_LOG_LEVEL_DEBUG,
                    "ESL FATFS Image: Image update started");
    }

    // decompression procedure will avoid overflow condition
    result = esl_image_unpack_chunk(data, length,
                                    chunk_data, 0,
                                    sizeof(chunk_data),
                                    &decompressed_size);

    if (SL_STATUS_OK != result) {
      sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                    ESL_LOG_LEVEL_ERROR,
                    "Image data receive failed (content or size mismatch)!");
      // invalidate the image in this case by clearing it's size
      active_image->size = 0;
      // return an error
      return SL_STATUS_ABORT;
    }

    char file_name[32];
    snprintf(file_name,
             sizeof(file_name),
             IMAGE_ROOT_DIR "/" IMAGE_NAME_PREFIX "%lu",
             active_image->image_index);

    FIL fil;
    FRESULT rc = f_open(&fil, file_name, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

    if (FR_OK != rc) {
      sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                    ESL_LOG_LEVEL_ERROR,
                    "ESL FATFS Image: FATFS open file operation failed with status %d!",
                    rc);
      return SL_STATUS_NO_MORE_RESOURCE;
    }

    rc = f_lseek(&fil, offset);
    if (FR_OK != rc) {
      sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                    ESL_LOG_LEVEL_ERROR,
                    "ESL FATFS Image: FATFS seek operation failed with status %d!",
                    rc);
      result = SL_STATUS_NO_MORE_RESOURCE;
      goto close;
    }

    UINT bw;
    rc = f_write(&fil, chunk_data, decompressed_size, &bw);
    if ((FR_OK != rc) || (bw != decompressed_size)) {
      sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                    ESL_LOG_LEVEL_ERROR,
                    "ESL FATFS Image: FATFS write operation failed with status %d!",
                    rc);
      result = SL_STATUS_NO_MORE_RESOURCE;
      goto close;
    }
    sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                  ESL_LOG_LEVEL_DEBUG,
                  "ESL FATFS Image: FATFS write %d bytes to offset: %d",
                  length,
                  offset);
    result = SL_STATUS_OK;

    close:
    // Close file
    rc = f_close(&fil);
    if (FR_OK != rc) {
      sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                    ESL_LOG_LEVEL_ERROR,
                    "ESL FATFS Image: FATFS close file operation failed with status %d!",
                    rc);
    }
    if (result != SL_STATUS_OK) {
      return result;
    }

    active_image->size += length;
    // check if this is the last expected image chunk - if it is, then
    // close our "poor man's linked list" by setting next key to zero
    if (active_image->size == active_image->max_size) {
      // improve OTS response by not sending credit after the slot is full
      result = SL_STATUS_FULL;
    }
  } else {
    sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                  ESL_LOG_LEVEL_ERROR,
                  "ESL FATFS Image: No image selected");
  }

  return result;
}

sl_status_t esl_image_select_object(void const *data, uint16_t length)
{
  // in case of size mismatch, this is the default answer:
  sl_status_t    result = SL_STATUS_INVALID_HANDLE;
  const uint16_t valid_size = sizeof(esl_image_object_id_t);

  // select null object by default (in case of error)
  image_registry.active_image = NULL;

  if (length == valid_size) {
    if (*(uint16_t *)data < ESL_IMAGE_OBJECT_BASE) {
      // invalid Object ID (wrong base?)
      result = SL_STATUS_INVALID_PARAMETER;
    } else if (*(uint8_t *)data >= image_registry.images_count) {
      // image index is out of bounds
      result = SL_STATUS_INVALID_INDEX;
    } else {
      // select active image storage for the upcoming data transfer
      image_registry.active_image = &image_object[*(uint8_t *)data];
      result = SL_STATUS_OK;
    }
  }

  return result;
}

void esl_image_init(void)
{
  // clear all images data, ESL Profile specification d09r18, Section 3.2
//  memset(image_pool, 0, sizeof(image_pool));

  // initialize zero images count, set max. available size for image allocation
  image_registry.active_image = NULL;
//  image_registry.next_object  = (void *)image_pool;
  image_registry.images_count = 0;
}

void esl_image_characteristic_update(void)
{
  sl_status_t sc;
  (void)sc; // suppress the compiler warning if sl_bt_esl_assert disabled
  uint8_t gatt_data = image_registry.images_count; // ESL Serv.spec. 3.7.1

  // gattdb_esl_image_info shall contain Max_Image_Index, not the image count!
  if (gatt_data > 0) {
    gatt_data -= 1;
  }

  sc = sl_bt_gatt_server_write_attribute_value(gattdb_esl_image_info,
                                               0,
                                               sizeof(gatt_data),
                                               &gatt_data);
  sl_bt_esl_assert(sc == SL_STATUS_OK);
}

sl_status_t esl_image_add(uint16_t width, uint16_t height,
                          uint8_t bits_per_pixel,
                          sl_bt_ots_object_type_t *type)
{
  sl_status_t result = SL_STATUS_INVALID_STATE;
  esl_state_t state = esl_core_get_status();

  // ESL Image: adding images after ESL boot event is disallowed!
  sl_bt_esl_assert(esl_state_boot == state);

  if (state == esl_state_boot) {
    result = SL_STATUS_NO_MORE_RESOURCE;

    // Refuse to add any new image if the max images count is reached!
    if (image_registry.images_count < ESL_TAG_MAX_IMAGES) {
      // get image size in bytes
      uint32_t  size = (bits_per_pixel * width * height) / 8;

      result = SL_STATUS_ALLOCATION_FAILED;

      uint8_t new_image_index = image_registry.images_count;

      // store maximum size of the image object
      image_object[new_image_index].max_size = (uint16_t)size;
      // set actual size to zero, initially
      image_object[new_image_index].size = 0;
      // allocate raw data pointer in the pool
      image_object[new_image_index].image_index = new_image_index;
      // finish registration
      ++image_registry.images_count;
      result = esl_tag_ots_add_object(type, size, ESL_OTS_IMAGE_OBJECT);
    } else {
      sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                    ESL_LOG_LEVEL_ERROR,
                    "Unable to add more images, ESL_TAG_MAX_IMAGES might be low.");
    }
  }

  return result;
}

sl_status_t esl_image_get_data(uint8_t image_index, uint16_t *offset,
                               uint16_t buf_size, uint8_t *target_buf)
{
  sl_status_t result = SL_STATUS_INVALID_INDEX;

  // check if given index points to a registered image
  if (image_index < image_registry.images_count) {
    // calculate the remaining size to read
    uint16_t remaining_size = image_object[image_index].size - *offset;

    // check if image has valid data
    if (image_object[image_index].size == 0) {
      // set proper ESL ERROR to be sent to the ESL Access point
      esl_core_set_last_error(ESL_ERROR_IMAGE_NOT_AVAILABLE);
      return SL_STATUS_NOT_AVAILABLE;
    } else if (*offset > image_object[image_index].size) {
      // else check if offset is out of bounds
      esl_core_set_last_error(ESL_ERROR_UNSPECIFIED);
      return SL_STATUS_ABORT;
    } else if (*offset == image_object[image_index].size) {
      // else check an edge case: read to be finished just right now
      return SL_STATUS_OK;
    } else if (remaining_size > buf_size) {
      // or adjust remaining size to the buffer size on success
      remaining_size = buf_size;
    }

    // if there's still data to read
    if (remaining_size != 0) {
      char file_name[32];
      snprintf(file_name,
               sizeof(file_name),
               IMAGE_ROOT_DIR "/" IMAGE_NAME_PREFIX "%d",
               image_index);

      // Open file to read
      FIL fil;
      FRESULT rc = f_open(&fil, file_name, FA_OPEN_EXISTING | FA_READ);
      if (FR_OK != rc) {
        sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                      ESL_LOG_LEVEL_ERROR,
                      "ESL FATFS Image: FATFS open file operation failed with status %d!",
                      rc);
        esl_core_set_last_error(ESL_ERROR_IMAGE_NOT_AVAILABLE);
        return SL_STATUS_NOT_AVAILABLE;
      }

      rc = f_lseek(&fil, *offset);
      if (FR_OK != rc) {
        sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                      ESL_LOG_LEVEL_ERROR,
                      "ESL FATFS Image: FATFS seek operation failed with status %d!",
                      rc);
        esl_core_set_last_error(ESL_ERROR_UNSPECIFIED);
        result = SL_STATUS_ABORT;
        goto close;
      }

      // Read image chunk from file
      UINT br;
      rc = f_read(&fil, target_buf, remaining_size, &br);
      if ((FR_OK != rc) || (remaining_size != br)) {
        sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                      ESL_LOG_LEVEL_ERROR,
                      "ESL FATFS Image: FATFS read operation failed with status %d!",
                      rc);
        result = SL_STATUS_FAIL;
        goto close;
      }
      sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                    ESL_LOG_LEVEL_DEBUG,
                    "ESL FATFS Image: FATFS read %d bytes from offset: %d",
                    buf_size,
                    *offset);

      // shift the offset
      *offset += remaining_size;
      result = SL_STATUS_OK;
      close:
      // Close file
      rc = f_close(&fil);
      if (FR_OK != rc) {
        sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                      ESL_LOG_LEVEL_ERROR,
                      "ESL FATFS Image: FATFS close file operation failed with status %d!",
                      rc);
      }
      return result;
    }
    result = SL_STATUS_OK;
  } else {
    // signal invalid index to the remote ESL Access Point
    esl_core_set_last_error(ESL_ERROR_INVALID_IMAGE_INDEX);
    sl_bt_esl_log(ESL_LOG_COMPONENT_NVM_IMAGE,
                  ESL_LOG_LEVEL_ERROR,
                  "ESL FATFS Image: Invalid image index");
  }
  return result;
}

uint8_t esl_image_get_count()
{
  return image_registry.images_count;
}

void esl_image_reset_storage(void)
{
  uint8_t image_index = image_registry.images_count;

  while (image_index--) {
    // get every image objects one by one
    esl_image_object_t *image = &image_object[image_index];
    // invalidate data content by clearing the size
    image->size = 0;
  }
}
