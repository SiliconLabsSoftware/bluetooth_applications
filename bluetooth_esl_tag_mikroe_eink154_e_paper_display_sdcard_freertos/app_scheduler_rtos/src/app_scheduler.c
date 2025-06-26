/*
 * app_scheduler_rtos.c
 *
 *  Created on: Apr 18, 2025
 *      Author: vile
 */
#include <string.h>
#include "sl_common.h"
#include "sl_core.h"
#include "app_scheduler.h"

#define APP_SCHEDULER_TASK_PRIORITY     (51)
#define APP_SCHEDULER_TASK_STACK_SIZE   (2000)

#define ms_to_tick(ms) \
  ((uint32_t)(ms) * osKernelGetTickFreq() / 1000)
#define tick_to_ms(ticks) \
  (uint32_t)((uint64_t)ticks * 1000 / osKernelGetTickFreq())

#if !APP_SCHEDULER_TASK_HANDLER_ON_BT_THREAD
static const osThreadAttr_t thread_attr = {
  .name = "App scheduler stack",
  .stack_size = APP_SCHEDULER_TASK_STACK_SIZE,
  .priority = (osPriority_t) APP_SCHEDULER_TASK_PRIORITY
};
#endif

app_scheduler_entry_t entry_pool[APP_SCHEDULER_MAX_QUEUE_SIZE];
osMemoryPoolId_t pool_handle;

static osMemoryPoolAttr_t mem_pool_attr = {
  .name = "App scheduler pool",
  .cb_mem = NULL,
  .cb_size = 0,
  .mp_mem = entry_pool,
  .mp_size = APP_SCHEDULER_MAX_QUEUE_SIZE
};

app_scheduler_entry_t *entry_queue[APP_SCHEDULER_MAX_QUEUE_SIZE];
osMessageQueueId_t queue_handle;

osMessageQueueAttr_t queue_attr = {
  .name = "App scheduler queue",
  .cb_mem = NULL,
  .cb_size = 0,
  .mq_mem = entry_queue,
  .mq_size = APP_SCHEDULER_MAX_QUEUE_SIZE
};

// Start of the linked list which contains the queue
static sl_slist_node_t *task_queue = NULL;

// State of the scheduler
static bool run;

static void queue_changed_notify(void);

static void timer_callback(void *data)
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  if (((app_scheduler_entry_t *)data)->triggered == false) {
    ((app_scheduler_entry_t *)data)->triggered = true;
  }
  if (((app_scheduler_entry_t *)data)->periodic) {
    ((app_scheduler_entry_t *)data)->timer_start_tick = osKernelGetTickCount();
  }
  CORE_EXIT_CRITICAL();

  osMessageQueuePut(queue_handle, &data, 0, osWaitForever);
  queue_changed_notify();
}

/***************************************************************************//**
 * Add a task to be scheduled with optional data parameter
 ******************************************************************************/
static sl_status_t add_new_task(app_scheduler_task_t task,
                                uint32_t timeout_ms,
                                bool is_periodic,
                                void *data,
                                size_t size,
                                app_scheduler_task_handle_t *handle)
{
  app_scheduler_entry_t *entry = NULL;
  sl_status_t sc = SL_STATUS_OK;
  CORE_DECLARE_IRQ_STATE;

  // Check queue
  if (!osMessageQueueGetSpace(queue_handle)
      && !osMemoryPoolGetSpace(pool_handle)) {
    sc = SL_STATUS_NO_MORE_RESOURCE;
  }

  // Check parameters
  if (size > APP_SCHEDULER_MAX_DATA_SIZE) {
    sc = SL_STATUS_INVALID_PARAMETER;
  }
  if (is_periodic && (timeout_ms == 0)) {
    sc = SL_STATUS_INVALID_PARAMETER;
  }
  if (task == NULL) {
    sc = SL_STATUS_NULL_POINTER;
  }

  if (sc == SL_STATUS_OK) {
    entry = osMemoryPoolAlloc(pool_handle, osWaitForever);
    if (entry) {
      entry->task = task;
      entry->periodic = is_periodic;
      // Copy data
      if (data && size) {
        memcpy(entry->data, data, size);
      }
      entry->data_size = size;
      entry->timer_start_tick = osKernelGetTickCount();
      entry->timer_timeout_ms = timeout_ms;

      if (timeout_ms == 0) {
        // Make triggered
        CORE_ENTER_CRITICAL();
        entry->triggered = true;
        entry->timer_handle = NULL;
        sl_slist_push_back(&task_queue, &entry->node);
        CORE_EXIT_CRITICAL();
        osMessageQueuePut(queue_handle, &entry, 0, osWaitForever);
        queue_changed_notify();
      } else {
        // Make untriggered
        entry->triggered = false;

        entry->timer_handle = osTimerNew(timer_callback,
                                         is_periodic ? osTimerOnce : osTimerPeriodic,
                                         (void *)entry,
                                         NULL);
        osStatus_t oss = osError;

        if (entry->timer_handle) {
          // Start timer
          oss = osTimerStart(entry->timer_handle,
                             ms_to_tick(timeout_ms));
        }
        if (oss == osOK) {
          CORE_ENTER_CRITICAL();
          sl_slist_push_back(&task_queue, &entry->node);
          CORE_EXIT_CRITICAL();
        } else {
          // Free up memory
          osMemoryPoolFree(pool_handle, entry);
        }
      }

      if ((sc == SL_STATUS_OK) && (handle != NULL)) {
        *handle = (app_scheduler_task_handle_t)entry;
      }
    }
  }

  return sc;
}

void app_scheduler_task_handler(uint32_t timeout)
{
  app_scheduler_entry_t *entry = NULL;
  CORE_DECLARE_IRQ_STATE;

  while (osOK == osMessageQueueGet(queue_handle, &entry, 0, timeout)) {
    // Call task function
    entry->task(entry->data, entry->data_size);

    CORE_ENTER_CRITICAL();
    entry->triggered = false;
    CORE_EXIT_CRITICAL();

    // if not periodic, remove
    if (false == entry->periodic) {
      // Free up memory
      app_scheduler_remove((app_scheduler_task_handle_t)entry);
    }
  }
}

#if APP_SCHEDULER_TASK_HANDLER_ON_BT_THREAD
static void queue_changed_notify(void)
{
  sl_bt_external_signal(APP_SCHEDULER_BT_EVENT_EXT_SIGNAL_ID);
}

void app_scheduler_bt_on_event(sl_bt_msg_t *evt)
{
  if (SL_BT_MSG_ID(evt->header) == sl_bt_evt_system_external_signal_id) {
    if (evt->data.evt_system_external_signal.extsignals
        & APP_SCHEDULER_BT_EVENT_EXT_SIGNAL_ID) {
      app_scheduler_task_handler(0);
    }
  }
}

#else
static void queue_changed_notify(void)
{
}

void app_scheduler_bt_on_event(sl_bt_msg_t *evt)
{
  (void) evt;
}

static void app_scheduler_rtos_thread(void *data)
{
  (void) data;

  while (1) {
    app_scheduler_task_handler(osWaitForever);
  }
}

#endif

/******************************************************************************
 * Scheduler init
 ******************************************************************************/
sl_status_t app_scheduler_init(void)
{
  pool_handle = osMemoryPoolNew(APP_SCHEDULER_MAX_QUEUE_SIZE,
                                sizeof(app_scheduler_entry_t),
                                &mem_pool_attr);
  if (pool_handle == NULL) {
    return SL_STATUS_FAIL;
  }

  queue_handle = osMessageQueueNew(APP_SCHEDULER_MAX_QUEUE_SIZE,
                                   sizeof(app_scheduler_entry_t *),
                                   NULL);

  sl_slist_init(&task_queue);
  run = APP_SCHEDULER_ENABLE;

#if !APP_SCHEDULER_TASK_HANDLER_ON_BT_THREAD
  osThreadId_t thread_id = osThreadNew(app_scheduler_rtos_thread,
                                       NULL,
                                       &thread_attr);
  if (thread_id == NULL) {
    return SL_STATUS_FAIL;
  }
#endif

  return SL_STATUS_OK;
}

/******************************************************************************
 * Scheduler pause
 ******************************************************************************/
void app_scheduler_pause(void)
{
  run = false;
}

/******************************************************************************
 * Scheduler resume
 ******************************************************************************/
void app_scheduler_resume(void)
{
  run = true;
}

/***************************************************************************//**
 * Add a task to be scheduled with optional data parameter
 ******************************************************************************/
sl_status_t app_scheduler_add(app_scheduler_task_t task,
                              void *data,
                              size_t size,
                              app_scheduler_task_handle_t *handle)
{
  sl_status_t sc;
  sc = add_new_task(task, 0, false, data, size, handle);
  return sc;
}

/***************************************************************************//**
 * Remove a previously scheduled task
 ******************************************************************************/
sl_status_t app_scheduler_remove(app_scheduler_task_handle_t handle)
{
  app_scheduler_entry_t *task = NULL;
  sl_status_t sc = SL_STATUS_NOT_FOUND;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  // Linear search in the queue for the task
  SL_SLIST_FOR_EACH_ENTRY(task_queue, task, app_scheduler_entry_t, node) {
    if (((app_scheduler_entry_t *)handle) == task) {
      // Stop timer
      if (task->timer_handle) {
        if (osTimerIsRunning(task->timer_handle)) {
          osTimerStop(task->timer_handle);
        }
        osTimerDelete(task->timer_handle);
      }

      // Remove from list
      sl_slist_remove(&task_queue, &task->node);

      // Free up memory
      osMemoryPoolFree(pool_handle, task);

      sc = SL_STATUS_OK;
      // Quit the loop
      break;
    }
  }
  CORE_EXIT_CRITICAL();

  return sc;
}

/***************************************************************************//**
 * Add a periodic task to be scheduled with optional data parameter
 ******************************************************************************/
sl_status_t app_scheduler_add_periodic(app_scheduler_task_t task,
                                       uint32_t period_ms,
                                       void *data,
                                       size_t size,
                                       app_scheduler_task_handle_t *handle)
{
  sl_status_t sc;
  sc = add_new_task(task, period_ms, true, data, size, handle);
  return sc;
}

/***************************************************************************//**
 * Add a task to be scheduled with optional data parameter and a delay
 ******************************************************************************/
sl_status_t app_scheduler_add_delayed(app_scheduler_task_t task,
                                      uint32_t delay_ms,
                                      void *data,
                                      size_t size,
                                      app_scheduler_task_handle_t *handle)
{
  sl_status_t sc;
  sc = add_new_task(task, delay_ms, false, data, size, handle);
  return sc;
}

/***************************************************************************//**
 * Reschedule a previously scheduled task
 ******************************************************************************/
sl_status_t app_scheduler_reschedule(app_scheduler_task_handle_t handle,
                                     uint32_t delay_ms)
{
  app_scheduler_entry_t *task = NULL;
  sl_status_t sc = SL_STATUS_NOT_FOUND;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  // Linear search in the queue for the task
  SL_SLIST_FOR_EACH_ENTRY(task_queue, task, app_scheduler_entry_t, node) {
    if (((app_scheduler_entry_t *)handle) == task) {
      // Stop timer
      osTimerStop(task->timer_handle);

      // Start with the new timer
      if (osOK == osTimerStart(task->timer_handle,
                               ms_to_tick(delay_ms))) {
        sc = SL_STATUS_OK;
      } else {
        sc = SL_STATUS_FAIL;
      }
      // Quit the loop
      break;
    }
  }
  CORE_EXIT_CRITICAL();

  return sc;
}

/***************************************************************************//**
 * Get the remaining time in ms for a given, scheduled task
 ******************************************************************************/
sl_status_t app_scheduler_get_remaining_time_ms(
  app_scheduler_task_handle_t handle,
  uint32_t *time_ms)
{
  sl_status_t sc = SL_STATUS_NOT_FOUND;
  uint32_t elapsed_ms = 0;

  CORE_DECLARE_IRQ_STATE;
  app_scheduler_entry_t *entry = NULL;

  CORE_ENTER_CRITICAL();
  // Linear search in the queue for the task
  SL_SLIST_FOR_EACH_ENTRY(task_queue, entry, app_scheduler_entry_t, node) {
    if (((app_scheduler_entry_t *)handle) == entry) {
      elapsed_ms = tick_to_ms(osKernelGetTickCount() - entry->timer_start_tick);
      *time_ms = elapsed_ms < entry->timer_timeout_ms
                 ?entry->timer_timeout_ms - elapsed_ms : 0;
      break;
    }
  }

  CORE_EXIT_CRITICAL();

  return sc;
}

/***************************************************************************//**
 * Get the delay or period in ms for a given, scheduled task
 ******************************************************************************/
sl_status_t app_scheduler_get_timeout(app_scheduler_task_handle_t handle,
                                      uint32_t *delay_ms)
{
  app_scheduler_entry_t *task = NULL;
  sl_status_t sc = SL_STATUS_NOT_FOUND;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  // Linear search in the queue for the task
  SL_SLIST_FOR_EACH_ENTRY(task_queue, task, app_scheduler_entry_t, node) {
    if (((app_scheduler_entry_t *)handle) == task) {
      // Set the output and status code
      *delay_ms = task->timer_timeout_ms;
      sc = SL_STATUS_OK;

      // Quit the loop
      break;
    }
  }
  CORE_EXIT_CRITICAL();

  return sc;
}
