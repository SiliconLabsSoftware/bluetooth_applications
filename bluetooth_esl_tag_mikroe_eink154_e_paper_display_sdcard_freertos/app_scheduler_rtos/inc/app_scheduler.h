/*
 * app_scheduler_rtos.h
 *
 *  Created on: Apr 18, 2025
 *      Author: vile
 */

#ifndef RTOS_APP_SCHEDULER_H_
#define RTOS_APP_SCHEDULER_H_

#include <stddef.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_slist.h"
#include "cmsis_os2.h"
#include "sl_bt_api.h"
#include "app_scheduler_rtos_config.h"

typedef struct app_scheduler_entry *app_scheduler_task_handle_t;

/***************************************************************************//**
 * Expected prototype of the user's task function to be scheduled.
 *
 * @param[in] data Data that is passed to the task.
 * @param[in] size Size of the data.
 ******************************************************************************/
typedef void (*app_scheduler_task_t)(void *data, uint16_t size);

/// Scheduler entry structure
typedef struct {
  sl_slist_node_t node;                                     ///< List node.
  app_scheduler_task_t task;                                ///< Task function.
  osTimerId_t timer_handle;                                 ///< Timer handle.
  uint32_t timer_start_tick;
  uint32_t timer_timeout_ms;
  bool triggered;                                           ///< Timer is triggered.
  bool periodic;                                            ///< Timer is periodic.
  size_t data_size;                                         ///< Size of the data
  uint8_t data[APP_SCHEDULER_MAX_DATA_SIZE];                ///< Data to pass
} app_scheduler_entry_t;

/***************************************************************************//**
 * Add a task to be scheduled with optional data parameter
 *
 * @param[in] task Task function to be scheduled
 * @param[in] data The data that is passed to the task.
 * @param[in] size The size of the data.
 * @param[out] handle Handle of the task
 *
 * @retval SL_STATUS_NO_MORE_RESOURCE No more space in queue
 *                                    or no memory is available
 * @retval SL_STATUS_NULL_POINTER Null pointer specified for data
 * @retval SL_STATUS_INVALID_PARAMETER Data is too long
 * @retval SL_STATUS_OK Operation was successful
 * @return Status of the operation.
 ******************************************************************************/
sl_status_t app_scheduler_add(app_scheduler_task_t task,
                              void *data,
                              size_t size,
                              app_scheduler_task_handle_t *handle);

/***************************************************************************//**
 * Remove a previously scheduled task
 *
 * @param[in] handle Handle of the task to be removed
 *
 * @retval SL_STATUS_NOT_FOUND task not found
 * @retval SL_STATUS_OK Operation was successful
 * @return Status of the operation.
 ******************************************************************************/
sl_status_t app_scheduler_remove(app_scheduler_task_handle_t handle);

/***************************************************************************//**
 * Add a task to be scheduled with optional data parameter
 *
 * @param[in] task Task function to be scheduled
 * @param[in] delay_ms Initial delay in ms.
 * @param[in] data The data that is passed to the task.
 * @param[in] size The size of the data.
 * @param[out] handle Handle of the task. NULL can be specified if
 *
 * @retval SL_STATUS_NO_MORE_RESOURCE No more space in queue
 *                                    or no memory is available
 * @retval SL_STATUS_NULL_POINTER Null pointer specified for data
 * @retval SL_STATUS_INVALID_PARAMETER Data is too long
 * @retval SL_STATUS_OK Operation was successful
 * @return Status of the operation.
 ******************************************************************************/
sl_status_t app_scheduler_add_delayed(app_scheduler_task_t task,
                                      uint32_t delay_ms,
                                      void *data,
                                      size_t size,
                                      app_scheduler_task_handle_t *handle);

/***************************************************************************//**
 * Add a periodic task to be scheduled with optional data parameter
 *
 * @param[in] task Task function to be scheduled
 * @param[in] period_ms Period in ms.
 * @param[in] data The data that is passed to the task.
 * @param[in] size The size of the data.
 * @param[out] handle Handle of the task
 *
 * @retval SL_STATUS_NO_MORE_RESOURCE No more space in queue
 *                                    or no memory is available
 * @retval SL_STATUS_NULL_POINTER Null pointer specified for data
 * @retval SL_STATUS_INVALID_PARAMETER Data is too long
 * @retval SL_STATUS_OK Operation was successful
 * @return Status of the operation.
 ******************************************************************************/
sl_status_t app_scheduler_add_periodic(app_scheduler_task_t task,
                                       uint32_t period_ms,
                                       void *data,
                                       size_t size,
                                       app_scheduler_task_handle_t *handle);

/***************************************************************************//**
 * Get the delay or period in ms for a given, scheduled task
 *
 * @param[in] handle   Handle of the task to be queried
 * @param[out] delay_ms Output delay or period
 *
 * @retval SL_STATUS_NOT_FOUND task not found
 * @retval SL_STATUS_OK Operation was successful
 * @return Status of the operation.
 ******************************************************************************/
sl_status_t app_scheduler_get_timeout(app_scheduler_task_handle_t handle,
                                      uint32_t *delay_ms);

/***************************************************************************//**
 * Reschedule a previously scheduled task
 *
 * The scheduling starts over with the new value. If rescheduling has to take
 * the remaining time into account then the necessary calculation shall take
 * place by the application itself. This procedure may affect the accuracy
 * depending on the actual implementation. The @ref `app_scheduler_get_timeout`
 * API is provided for getting the remaining time for an already scheduled
 * task.
 *
 * @param[in] handle   Handle of the task to be rescheduled
 * @param[in] delay_ms New delay or period
 *
 * @retval SL_STATUS_NOT_FOUND task not found
 * @retval SL_STATUS_OK Operation was successful
 * @return Status of the operation.
 ******************************************************************************/
sl_status_t app_scheduler_reschedule(app_scheduler_task_handle_t handle,
                                     uint32_t delay_ms);

/***************************************************************************//**
 * Initialize the scheduler
 ******************************************************************************/
sl_status_t app_scheduler_init(void);

void app_scheduler_bt_on_event(sl_bt_msg_t *evt);

#endif /* RTOS_APP_SCHEDULER_H_ */
