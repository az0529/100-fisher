#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__

#include "esp_err.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "task_comm.h"

// 引脚定义
#define TRIG_GPIO 17  // 触发引脚
#define ECHO_GPIO 18  // 回声引脚

// 最大测量距离（单位：厘米）
#define MAX_DISTANCE 600
// 盲区（单位：厘米）
#define MIN_DISTANCE 20

// 函数声明
esp_err_t ultrasonic_init(void);
float ultrasonic_measure_distance(void);
float ultrasonic_measure_distance_fast(void);
uint32_t ultrasonic_get_raw_pulse_width(void);
esp_err_t ultrasonic_detect_motion_density(uint32_t sample_time, uint32_t sample_interval, float threshold, int *density, float *frequency);
void ultrasonic_add_to_cache(float distance);
int ultrasonic_get_cache(float *data, int size);
esp_err_t ultrasonic_start_30s_sampling(void);
esp_err_t ultrasonic_get_sampling_result(int *density, float *motion_change);
esp_err_t ultrasonic_stop_sampling(void);
esp_err_t ultrasonic_deinit(void);
QueueHandle_t ultrasonic_get_distance_queue(void);

#endif