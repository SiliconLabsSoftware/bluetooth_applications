/***************************************************************************//**
 * @file display_and_sound.c
 * @brief pulse_oximeter_and_heart_rate_monitor display and sound source
 * @version 1.0
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
#include <display_and_sound.h>
#include "stdio.h"
#include "micro_oled_ssd1306.h"
#include "glib.h"
#include "glib_font.h"
#include "sl_i2cspm_instances.h"
#include "app_log.h"
#include "app_assert.h"
#include "mikroe_cmt_8540s_smt.h"
#include "sl_pwm_instances.h"
#include "pulse_oximeter_and_heart_rate_monitor.h"

#define BUZZER_IMG_WIDTH                            (19)
#define BUZZER_IMG_HEIGHT                           (17)
#define BUZZER_IMG_X_POSITION                       (22)
#define BUZZER_IMG_Y_POSITION                       (0)

#define NO_FINGER_DETECTED_IMG_WIDTH                (49)
#define NO_FINGER_DETECTED_IMG_HEIGHT               (46)
#define NO_FINGER_DETECTED_IMG_X_POSITION           (7)
#define NO_FINGER_DETECTED_IMG_Y_POSITION           (1)

#define RELEASE_PRESSURE_IMG_WIDTH                  (46)
#define RELEASE_PRESSURE_IMG_HEIGHT                 (46)
#define RELEASE_PRESSURE_IMG_X_POSITION             (9)
#define RELEASE_PRESSURE_IMG_Y_POSITION             (1)

#define SPO2_X_POSITION                             (0)
#define SPO2_Y_POSITION                             (20)
#define HEART_RATE_X_POSITION                       (35)
#define HEART_RATE_Y_POSITION                       (20)

#define SPO2_TEXT_X_POSITION                        (13)
#define SPO2_TEXT_Y_POSITION                        (26)
#define PR_TEXT_X_POSITION                          (54)
#define PR_TEXT_Y_POSITION                          (26)

#define LINE_X_START_POSITION                       (0)
#define LINE_Y_START_POSITION                       (37)
#define LINE_X_END_POSITION                         (63)
#define LINE_Y_END_POSITION                         (37)

#define CONFIDENCE_TEXT_X_POSITION                  (3)
#define CONFIDENCE_TEXT_Y_POSITION                  (44)

#define BLINK_BUZZER_START_X_POSITION               (20)
#define BLINK_BUZZER_START_Y_POSITION               (0)
#define BLINK_BUZZER_END_X_POSITION                 (43)
#define BLINK_BUZZER_END_Y_POSITION                 (18)

#define BLINK_SPO2_START_X_POSITION                 (0)
#define BLINK_SPO2_START_Y_POSITION                 (18)
#define BLINK_SPO2_END_X_POSITION                   (30)
#define BLINK_SPO2_END_Y_POSITION                   (30)

#define BLINK_PR_START_X_POSITION                   (34)
#define BLINK_PR_START_Y_POSITION                   (18)
#define BLINK_PR_END_X_POSITION                     (65)
#define BLINK_PR_END_Y_POSITION                     (30)

#define BLINK_INTERVAL_MS                           (200)
#define MAX_SPO2_VAL                                (100)

#define BUZZER_VOLUME_SCALE_FACTOR                  (10)
#define BUZZER_PLAY_TIME_MS                         (200)

typedef enum {
  STATE_NO_FINGER_DETECTED = 0,
  STATE_RELEASE_PRESSURE = 1,
  STATE_NORMAL  = 2,
  STATE_ALARM = 3,
}display_state_t;

static const uint8_t no_finger_detected_img[] = {
  0x00, 0xf8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x1f, 0x00, 0x00,
  0x00, 0x00, 0x80, 0x07, 0x3c, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf1, 0xf1,
  0x00, 0x00, 0x00, 0x00, 0xe0, 0xfc, 0xc7, 0x01, 0x00, 0x00, 0x00, 0x30,
  0x0f, 0x9e, 0x03, 0x00, 0x00, 0x00, 0xb8, 0x03, 0x38, 0x03, 0x00, 0x00,
  0x00, 0xd8, 0xf9, 0x73, 0x06, 0x00, 0x00, 0x00, 0xcc, 0xfc, 0xe7, 0x0e,
  0x00, 0x00, 0x00, 0x6c, 0x0e, 0xcc, 0x0c, 0x00, 0x00, 0x00, 0x66, 0xe3,
  0x98, 0x0d, 0x00, 0x00, 0x00, 0x36, 0xf3, 0xb3, 0x19, 0x00, 0x00, 0x00,
  0xb6, 0x19, 0x37, 0x1b, 0x00, 0x00, 0x00, 0x93, 0x4d, 0x66, 0x1b, 0x00,
  0x00, 0x00, 0x9b, 0xec, 0x6c, 0x1b, 0x00, 0x00, 0x00, 0xd8, 0xe4, 0x6d,
  0x13, 0x00, 0x00, 0x00, 0xcc, 0xa6, 0x6d, 0x13, 0x00, 0x00, 0x00, 0x6c,
  0xb6, 0x6d, 0x13, 0x00, 0x00, 0x00, 0x66, 0xb2, 0x6d, 0x1b, 0x00, 0x00,
  0x00, 0x33, 0x93, 0x04, 0x00, 0x00, 0x00, 0x00, 0xb8, 0x1b, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x9c, 0x19, 0xff, 0xff, 0x01, 0x00, 0x00, 0xce, 0x8c,
  0x03, 0xc0, 0x03, 0x00, 0x00, 0x66, 0x8e, 0x01, 0x00, 0x07, 0x00, 0x00,
  0x70, 0xb6, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x1c, 0xb3, 0x01, 0x00, 0xfc,
  0x1f, 0x00, 0x8e, 0x19, 0xff, 0x1f, 0xf0, 0x7f, 0x00, 0xe0, 0x0c, 0xfe,
  0x1f, 0x00, 0x60, 0x00, 0x70, 0xce, 0x81, 0x31, 0x00, 0xc0, 0x00, 0x1c,
  0xf7, 0x87, 0x61, 0x00, 0x80, 0x00, 0x88, 0x39, 0x8e, 0x61, 0x00, 0x80,
  0x01, 0xe0, 0x1c, 0x38, 0xff, 0x00, 0x80, 0x01, 0x70, 0xce, 0x73, 0xff,
  0x01, 0x80, 0x01, 0x10, 0x67, 0xa7, 0x01, 0x03, 0x80, 0x01, 0xc0, 0x31,
  0x8e, 0x01, 0x0e, 0x80, 0x01, 0xc0, 0x9c, 0x98, 0x03, 0x3c, 0x80, 0x01,
  0x00, 0xce, 0x03, 0xff, 0x31, 0x80, 0x01, 0x00, 0xe3, 0x07, 0xff, 0x03,
  0x80, 0x01, 0x00, 0x30, 0x80, 0x01, 0x02, 0x80, 0x01, 0x00, 0x10, 0x80,
  0x01, 0x02, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0xff, 0x01, 0x60, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x70,
  0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0xfc,
  0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t release_pressure_img[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x01, 0x00, 0x00,
  0x00, 0xc0, 0x00, 0x03, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x03, 0x00, 0x00,
  0x00, 0x80, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x07, 0x03, 0x00, 0x00,
  0x00, 0x60, 0x0e, 0x03, 0x00, 0x00, 0x00, 0xc0, 0x1c, 0x03, 0x00, 0x00,
  0x00, 0xc0, 0x39, 0x03, 0x00, 0x00, 0x00, 0x80, 0x73, 0x03, 0x00, 0x00,
  0x00, 0x00, 0x67, 0x03, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x03, 0x00, 0x00,
  0x00, 0x00, 0x1c, 0x03, 0x07, 0x00, 0x00, 0x00, 0x38, 0xc3, 0x3f, 0x00,
  0x00, 0x80, 0x71, 0xf3, 0xf8, 0x00, 0x00, 0x80, 0x61, 0x3b, 0xc0, 0x01,
  0x00, 0x80, 0x01, 0x1f, 0x80, 0x03, 0x00, 0xfc, 0x01, 0x7f, 0x00, 0x07,
  0x00, 0xfc, 0x01, 0x7f, 0x00, 0x0e, 0x00, 0x38, 0x00, 0x38, 0x00, 0x0c,
  0x00, 0x70, 0x00, 0x1c, 0x00, 0x18, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0x18,
  0x00, 0xc0, 0x01, 0x07, 0x00, 0x18, 0x00, 0x80, 0x83, 0x03, 0x00, 0x30,
  0x00, 0x00, 0xc7, 0x01, 0x00, 0x30, 0x00, 0x00, 0xe6, 0x00, 0x00, 0x18,
  0x00, 0x00, 0x7f, 0x00, 0x00, 0x18, 0x00, 0x80, 0x39, 0x00, 0x00, 0x18,
  0x00, 0xc0, 0x10, 0x00, 0x0c, 0x0c, 0x00, 0x60, 0xc0, 0x03, 0x0c, 0x0c,
  0x00, 0x30, 0xe0, 0x07, 0x06, 0x06, 0x00, 0x18, 0x70, 0x0c, 0x07, 0x03,
  0x00, 0x0c, 0x38, 0x06, 0x03, 0x03, 0x00, 0x06, 0x1c, 0xa6, 0x81, 0x01,
  0x00, 0x03, 0x0e, 0xe3, 0xc1, 0x00, 0x80, 0x01, 0x8f, 0xc3, 0xe0, 0x00,
  0xc0, 0x80, 0xfb, 0x61, 0x60, 0x00, 0xe0, 0xc0, 0xf9, 0x70, 0x30, 0x00,
  0x60, 0xe0, 0x80, 0x31, 0x38, 0x00, 0x60, 0x70, 0x80, 0x3f, 0x18, 0x00,
  0xe0, 0x38, 0x00, 0x3f, 0x0c, 0x00, 0xc0, 0x1f, 0x00, 0x70, 0x0e, 0x00,
  0x80, 0x0f, 0x00, 0xe0, 0x07, 0x00, 0xfc, 0xff, 0xff, 0x9f, 0x01, 0x00,
  0xfc, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t buzzer_img[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x80, 0xc7, 0x00,
  0xe0, 0x97, 0x01, 0xfe, 0x37, 0x01, 0xfe, 0x67, 0x01, 0xfe, 0x47, 0x03,
  0xfe, 0x47, 0x03, 0xfe, 0x47, 0x03, 0xfe, 0x47, 0x03, 0xfe, 0x67, 0x01,
  0xfc, 0xb7, 0x01, 0xc0, 0x87, 0x00, 0x80, 0xc7, 0x00, 0x00, 0x06, 0x00,
  0x00, 0x00, 0x00
};

static const uint8_t silicon_labs_logo_64x23[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x80, 0xff, 0x1f, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xf0,
  0xff, 0x7f, 0x00, 0x3e, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x7f, 0x00, 0x7f,
  0x00, 0x00, 0x00, 0xff, 0xff, 0x3f, 0x80, 0x7f, 0x00, 0x06, 0x80, 0xff,
  0xff, 0xff, 0xe0, 0xff, 0x80, 0x03, 0xc0, 0xff, 0x01, 0xf8, 0xff, 0x7f,
  0xe0, 0x01, 0xc0, 0x03, 0x00, 0x00, 0xfe, 0x7f, 0xf0, 0x01, 0x00, 0x01,
  0x00, 0x00, 0xf8, 0x3f, 0xf8, 0x03, 0x00, 0xf0, 0x07, 0x00, 0xe0, 0x3f,
  0xfc, 0x03, 0x00, 0x00, 0x20, 0x00, 0xe0, 0x1f, 0xfe, 0x0f, 0x00, 0x00,
  0xc0, 0x00, 0xe0, 0x07, 0xfe, 0xff, 0x00, 0x00, 0xfc, 0x00, 0xe0, 0x03,
  0xff, 0xc3, 0xff, 0xff, 0xff, 0x00, 0xf0, 0x00, 0x7e, 0x00, 0xfe, 0xff,
  0x7f, 0x00, 0x38, 0x00, 0x3e, 0x00, 0xff, 0xff, 0x3f, 0x00, 0x0c, 0x00,
  0x1c, 0x80, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x18, 0x00, 0xff, 0xff,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x1f, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

static glib_context_t glib_context;
static display_state_t  gdisplay_state = STATE_NO_FINGER_DETECTED;
static bool inverted_flag = false;
static bool blink_timer_trigger = false;
static sl_sleeptimer_timer_handle_t blink_timer_handle;
static bool buzzer_trigger_flag = false;
static sl_sleeptimer_timer_handle_t buzzer_active_timer_handle;

static void display_state(display_state_t state,
                          const pom_and_hr_monitor_data_t *data);
static void timer_blink_callback(sl_sleeptimer_timer_handle_t *handle,
                                 void *data);
static void timer_buzzer_callback(sl_sleeptimer_timer_handle_t *handle,
                                  void *data);
static void timer_blink_stop(void);
static void normal_color_draw(const pom_and_hr_monitor_data_t *data);
static void invert_color_draw(const pom_and_hr_monitor_data_t *data);
static void active_buzzer(bool active_status, const user_config_t *config);

extern uint16_t glib_get_pixel(glib_context_t *g_context, int16_t x, int16_t y);

sl_status_t display_and_sound_init(void)
{
  /* ---------- Initialize the display ---------- */
  sl_status_t ret_code = ssd1306_init(sl_i2cspm_qwiic);
  app_assert_status(ret_code);

  glib_init(&glib_context);

  // Fill lcd with background color
  glib_clear(&glib_context);

  glib_draw_xbitmap(&glib_context,
                    0, 12, silicon_labs_logo_64x23,
                    64, 23, GLIB_WHITE);
  glib_update_display();

  app_log("The display (SSD1306) init successfully\r\n");

  /* ---------- Initialize the Buzzer ---------- */
  ret_code = mikroe_cmt_8540s_smt_init(&sl_pwm_mikroe);
  app_assert_status(ret_code);

  mikroe_cmt_8540s_smt_set_duty_cycle(0.0);
  mikroe_cmt_8540s_smt_pwm_stop();
  app_log("The Buzzer init successfully\r\n");

  sl_sleeptimer_delay_millisecond(2000);

  return SL_STATUS_OK;
}

sl_status_t display_and_sound_process(const pom_and_hr_monitor_data_t *data,
                                      const user_config_t *config)
{
  if ((NULL == data) || (NULL == config)) {
    return SL_STATUS_NULL_POINTER;
  }

  static display_state_t  temp_state = STATE_NO_FINGER_DETECTED;

  if ((data->status.body == FINGER_DETECTED)
      && (data->status.body_ext == EXT_SUCCESS)) {
    if (data->status.spo2_alarm
        || data->status.low_heart_rate_alarm
        || data->status.high_heart_rate_alarm) {
      temp_state = STATE_ALARM;
    } else {
      temp_state = STATE_NORMAL;
    }
  } else if (data->status.body_ext == EXT_PRESSING_TOO_HARD) {
    temp_state = STATE_RELEASE_PRESSURE;
  } else {
    temp_state = STATE_NO_FINGER_DETECTED;
  }

  if (gdisplay_state != temp_state) {
    gdisplay_state = temp_state;

    if (gdisplay_state == STATE_ALARM) {
      blink_timer_trigger = true;
      inverted_flag = false;
      sl_sleeptimer_start_periodic_timer_ms(&blink_timer_handle,
                                            BLINK_INTERVAL_MS,
                                            timer_blink_callback,
                                            NULL,
                                            0,
                                            0);
      active_buzzer(true, config);
    } else {
      timer_blink_stop();
      active_buzzer(false, config);
    }
  }

  display_state(gdisplay_state, data);

  return SL_STATUS_OK;
}

static void display_state(display_state_t state,
                          const pom_and_hr_monitor_data_t *data)
{
  switch (state) {
    case STATE_NO_FINGER_DETECTED:
      glib_clear(&glib_context);
      glib_draw_xbitmap(&glib_context,
                        NO_FINGER_DETECTED_IMG_X_POSITION,
                        NO_FINGER_DETECTED_IMG_Y_POSITION,
                        no_finger_detected_img,
                        NO_FINGER_DETECTED_IMG_WIDTH,
                        NO_FINGER_DETECTED_IMG_HEIGHT,
                        GLIB_WHITE);
      break;
    case STATE_RELEASE_PRESSURE:
      glib_clear(&glib_context);
      glib_draw_xbitmap(&glib_context,
                        RELEASE_PRESSURE_IMG_X_POSITION,
                        RELEASE_PRESSURE_IMG_Y_POSITION,
                        release_pressure_img,
                        RELEASE_PRESSURE_IMG_WIDTH,
                        RELEASE_PRESSURE_IMG_HEIGHT,
                        GLIB_WHITE);
      break;

    case STATE_NORMAL:
      normal_color_draw(data);
      break;

    case STATE_ALARM:
      if (blink_timer_trigger) {
        blink_timer_trigger = false;

        if (inverted_flag == false) {
          normal_color_draw(data);
          glib_draw_xbitmap(&glib_context,
                            BUZZER_IMG_X_POSITION,
                            BUZZER_IMG_Y_POSITION,
                            buzzer_img,
                            BUZZER_IMG_WIDTH,
                            BUZZER_IMG_HEIGHT,
                            GLIB_WHITE);
        } else {
          invert_color_draw(data);
        }

        inverted_flag = !inverted_flag;
      }
      break;

    default:
      break;
  }
  glib_update_display();
}

static void timer_blink_callback(sl_sleeptimer_timer_handle_t *handle,
                                 void *data)
{
  (void) handle;
  (void) data;

  blink_timer_trigger = true;
}

static void timer_blink_stop(void)
{
  bool timer_is_running;

  sl_sleeptimer_is_timer_running(&blink_timer_handle, &timer_is_running);
  if (timer_is_running) {
    sl_sleeptimer_stop_timer(&blink_timer_handle);
  }
}

static void normal_color_draw(const pom_and_hr_monitor_data_t *data)
{
  uint8_t buffer[30];
  glib_clear(&glib_context);
  glib_set_font(&glib_context, NULL);

  snprintf((char *)buffer, sizeof(buffer), "%02d",
           (data->biodata.spo2
            < MAX_SPO2_VAL) ? data->biodata.spo2 : (MAX_SPO2_VAL - 1));
  glib_draw_string(&glib_context,
                   (const char *) buffer,
                   SPO2_X_POSITION,
                   SPO2_Y_POSITION);

  snprintf((char *)buffer, sizeof(buffer), "%03d", data->biodata.heart_rate);
  glib_draw_string(&glib_context,
                   (const char *) buffer,
                   HEART_RATE_X_POSITION,
                   HEART_RATE_Y_POSITION);

  glib_set_font(&glib_context, &glib_font_picopixel);
  snprintf((char *)buffer, sizeof(buffer), "SPO2");
  glib_draw_string(&glib_context,
                   (const char *) buffer,
                   SPO2_TEXT_X_POSITION,
                   SPO2_TEXT_Y_POSITION);
  snprintf((char *)buffer, sizeof(buffer), "PR");
  glib_draw_string(&glib_context,
                   (const char *) buffer,
                   PR_TEXT_X_POSITION,
                   PR_TEXT_Y_POSITION);

  glib_draw_line(&glib_context,
                 LINE_X_START_POSITION,
                 LINE_Y_START_POSITION,
                 LINE_X_END_POSITION,
                 LINE_Y_END_POSITION,
                 GLIB_WHITE);

  snprintf((char *)buffer,
           sizeof(buffer),
           "CONFIDENCE: %02d%%",
           data->biodata.confidence);
  glib_draw_string(&glib_context,
                   (const char *)buffer,
                   CONFIDENCE_TEXT_X_POSITION,
                   CONFIDENCE_TEXT_Y_POSITION);
  glib_update_display();
}

static void invert_color_draw(const pom_and_hr_monitor_data_t *data)
{
  bool pixel_data;

  // Buzzer invert color
  for (uint8_t x = BLINK_BUZZER_START_X_POSITION;
       x < BLINK_BUZZER_END_X_POSITION; x++) {
    for (uint8_t y = BLINK_BUZZER_START_Y_POSITION;
         y < BLINK_BUZZER_END_Y_POSITION; y++) {
      pixel_data = (bool)glib_get_pixel(&glib_context, x, y);
      pixel_data = !pixel_data;
      glib_draw_pixel(&glib_context, x, y, pixel_data);
    }
  }

  // SPO2 invert color
  if (data->status.spo2_alarm) {
    for (uint8_t x = BLINK_SPO2_START_X_POSITION; x < BLINK_SPO2_END_X_POSITION;
         x++) {
      for (uint8_t y = BLINK_SPO2_START_Y_POSITION;
           y < BLINK_SPO2_END_Y_POSITION; y++) {
        pixel_data = (bool)glib_get_pixel(&glib_context, x, y);
        pixel_data = !pixel_data;
        glib_draw_pixel(&glib_context, x, y, pixel_data);
      }
    }
  }

  if (data->status.high_heart_rate_alarm || data->status.low_heart_rate_alarm) {
    // PR invert color
    for (uint8_t x = BLINK_PR_START_X_POSITION; x < BLINK_PR_END_X_POSITION;
         x++) {
      for (uint8_t y = BLINK_PR_START_Y_POSITION; y < BLINK_PR_END_Y_POSITION;
           y++) {
        pixel_data = (bool)glib_get_pixel(&glib_context, x, y);
        pixel_data = !pixel_data;
        glib_draw_pixel(&glib_context, x, y, pixel_data);
      }
    }
  }

  glib_update_display();
}

static void active_buzzer(bool active_status, const user_config_t *config)
{
  if (active_status == true) {
    mikroe_cmt_8540s_smt_pwm_start();
    mikroe_cmt_8540s_smt_set_duty_cycle(config->buzzer_volume
                                        / BUZZER_VOLUME_SCALE_FACTOR);

    sl_sleeptimer_start_periodic_timer_ms(&buzzer_active_timer_handle,
                                          BUZZER_PLAY_TIME_MS,
                                          timer_buzzer_callback,
                                          (void *) config,
                                          0,
                                          0);
  } else {
    mikroe_cmt_8540s_smt_pwm_stop();
    buzzer_trigger_flag = false;
    bool timer_running = false;

    sl_sleeptimer_is_timer_running(&buzzer_active_timer_handle, &timer_running);
    if (timer_running) {
      sl_sleeptimer_stop_timer(&buzzer_active_timer_handle);
    }
  }
}

static void timer_buzzer_callback(sl_sleeptimer_timer_handle_t *handle,
                                  void *data)
{
  (void) handle;
  const user_config_t *config = (const user_config_t *) data;
  buzzer_trigger_flag = !buzzer_trigger_flag;

  if (buzzer_trigger_flag && (gdisplay_state == STATE_ALARM)) {
    mikroe_cmt_8540s_smt_pwm_start();
    mikroe_cmt_8540s_smt_set_duty_cycle(config->buzzer_volume
                                        / BUZZER_VOLUME_SCALE_FACTOR);
  } else {
    mikroe_cmt_8540s_smt_pwm_stop();
  }
}
