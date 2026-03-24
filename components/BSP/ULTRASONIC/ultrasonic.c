#include "ultrasonic.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <math.h>

static const char *TAG = "ultrasonic";
static uint32_t last_measure_time = 0;
static float last_valid_distance = -1.0;

// 覆盖式缓存（循环缓冲区）
#define CACHE_SIZE 10
static float distance_cache[CACHE_SIZE];
static uint32_t cache_index = 0;
static uint32_t cache_count = 0;
static SemaphoreHandle_t cache_mutex = NULL;
static TaskHandle_t sampling_task_handle = NULL;
static QueueHandle_t distance_queue = NULL;

/**
 * @brief 初始化超声波模块
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 * @description 初始化TRIG和ECHO引脚
 */
esp_err_t ultrasonic_init(void)
{
    ESP_LOGI(TAG, "Initializing ultrasonic sensor...");
    ESP_LOGI(TAG, "TRIG GPIO: %d, ECHO GPIO: %d", TRIG_GPIO, ECHO_GPIO);
    ESP_LOGI(TAG, "Mode 1: Pulse width output mode");
    ESP_LOGI(TAG, "Range: %d-%d cm", MIN_DISTANCE, MAX_DISTANCE);

    // 配置TRIG引脚为输出
    gpio_config_t trig_config = {
        .pin_bit_mask = (1ULL << TRIG_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    if (gpio_config(&trig_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure TRIG pin");
        return ESP_FAIL;
    }

    // 配置ECHO引脚为输入
    gpio_config_t echo_config = {
        .pin_bit_mask = (1ULL << ECHO_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, // 启用上拉电阻
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    if (gpio_config(&echo_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ECHO pin");
        return ESP_FAIL;
    }

    // 初始状态：TRIG低电平
    gpio_set_level(TRIG_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(200)); // 进一步减少稳定时间

    // 测试引脚状态
    int trig_level = gpio_get_level(TRIG_GPIO);
    int echo_level = gpio_get_level(ECHO_GPIO);
    ESP_LOGI(TAG, "Initial pin levels - TRIG: %d, ECHO: %d", trig_level, echo_level);

    // 初始化缓存互斥锁
    cache_mutex = xSemaphoreCreateMutex();
    if (cache_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create cache mutex");
        return ESP_FAIL;
    }

    // 初始化距离队列
    distance_queue = xQueueCreate(10, sizeof(float));
    if (distance_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create distance queue");
        return ESP_FAIL;
    }

    // 初始化缓存
    memset(distance_cache, 0, sizeof(distance_cache));
    cache_index = 0;
    cache_count = 0;

    ESP_LOGI(TAG, "Ultrasonic sensor initialized");
    return ESP_OK;
}

/**
 * @brief 获取原始ECHO脉冲宽度
 * @return uint32_t - 脉冲宽度，单位：微秒
 * @description 快速获取ECHO脉冲宽度，不进行距离计算
 */
uint32_t ultrasonic_get_raw_pulse_width(void)
{
    // 检查测量周期，确保≥60ms
    uint32_t current_time = esp_timer_get_time();
    if (current_time - last_measure_time < 60000) { // 60ms
        return 0;
    }

    // 确保TRIG引脚初始为低电平
    gpio_set_level(TRIG_GPIO, 0);
    esp_rom_delay_us(10); // 最小稳定时间

    // 发送触发信号（刚好10μs高电平，满足协议要求）
    gpio_set_level(TRIG_GPIO, 1);
    esp_rom_delay_us(10); // 10μs高电平
    gpio_set_level(TRIG_GPIO, 0);

    // 等待ECHO引脚变为高电平（快速超时）
    int timeout = 0;
    while (gpio_get_level(ECHO_GPIO) == 0) {
        if (timeout++ > 10000) { // 快速超时
            last_measure_time = esp_timer_get_time();
            return 0;
        }
        esp_rom_delay_us(1);
    }

    // 开始计时
    uint32_t start_time = esp_timer_get_time();

    // 等待ECHO引脚变为低电平（快速超时）
    timeout = 0;
    while (gpio_get_level(ECHO_GPIO) == 1) {
        if (timeout++ > 360000) { // 600cm的最大时间
            last_measure_time = esp_timer_get_time();
            return 0;
        }
        esp_rom_delay_us(1);
    }

    // 结束计时
    uint32_t end_time = esp_timer_get_time();
    last_measure_time = end_time;

    // 计算脉冲宽度（微秒）
    uint32_t pulse_width = end_time - start_time;
    return pulse_width;
}

/**
 * @brief 快速测量距离
 * @return float - 距离值，单位：厘米
 * @description 快速测量距离，用于实时显示
 */
float ultrasonic_measure_distance_fast(void)
{
    // 获取原始脉冲宽度
    uint32_t pulse_width = ultrasonic_get_raw_pulse_width();
    if (pulse_width == 0) {
        return last_valid_distance;
    }

    // 使用标准公式计算距离：cm = t / 58
    float distance = pulse_width / 58.0;

    // 检查距离是否在有效范围内
    if (distance >= MIN_DISTANCE && distance <= MAX_DISTANCE) {
        last_valid_distance = distance;
        return distance;
    }

    // 不在可测范围内，返回错误
    last_valid_distance = -1.0;
    return -1.0;
}

/**
 * @brief 测量距离
 * @return float - 距离值，单位：厘米
 * @description 发送触发信号，测量回声时间，计算距离
 */
float ultrasonic_measure_distance(void)
{
    // 快速测量
    float distance = ultrasonic_measure_distance_fast();
    
    // 只有在需要详细日志时才输出
    if (distance >= 0) {
        ESP_LOGI(TAG, "Distance: %.2f cm", distance);
    }
    
    return distance;
}

/**
 * @brief 中值滤波函数
 * @param data 数据数组
 * @param size 数组大小
 * @return float 中值
 */
static float median_filter(float *data, int size)
{
    // 复制数据到临时数组
    float temp[size];
    memcpy(temp, data, size * sizeof(float));
    
    // 冒泡排序
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (temp[j] > temp[j + 1]) {
                float swap = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = swap;
            }
        }
    }
    
    // 返回中值
    if (size % 2 == 0) {
        return (temp[size/2 - 1] + temp[size/2]) / 2.0;
    } else {
        return temp[size/2];
    }
}

/**
 * @brief 检测运动物体的密集程度
 * @param sample_time 采样时间（毫秒）
 * @param sample_interval 采样间隔（毫秒）
 * @param threshold 距离阈值（厘米）
 * @param density 密度指针（返回值：0-低，1-中，2-高）
 * @param frequency 频率指针（返回值：物体/秒）
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 * @description 对一段时间的数据进行多次采样，使用中值滤波过滤干扰，计算物体经过的时间和频率
 */
esp_err_t ultrasonic_detect_motion_density(uint32_t sample_time, uint32_t sample_interval, float threshold, int *density, float *frequency)
{
    if (density == NULL || frequency == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return ESP_FAIL;
    }
    
    int sample_count = sample_time / sample_interval;
    if (sample_count < 5) {
        ESP_LOGE(TAG, "Sample count too small");
        return ESP_FAIL;
    }
    
    // 存储采样数据
    float samples[sample_count];
    uint32_t timestamps[sample_count];
    
    // 开始采样
    ESP_LOGI(TAG, "Starting motion detection with %d samples over %d ms", sample_count, sample_time);
    
    for (int i = 0; i < sample_count; i++) {
        samples[i] = ultrasonic_measure_distance_fast();
        timestamps[i] = esp_timer_get_time() / 1000; // 转换为毫秒
        vTaskDelay(pdMS_TO_TICKS(sample_interval));
    }
    
    // 应用中值滤波
    float filtered_samples[sample_count];
    const int filter_window = 3; // 滤波窗口大小
    
    for (int i = 0; i < sample_count; i++) {
        if (i < filter_window/2 || i >= sample_count - filter_window/2) {
            // 边缘数据直接使用
            filtered_samples[i] = samples[i];
        } else {
            // 中值滤波
            float window[filter_window];
            for (int j = 0; j < filter_window; j++) {
                window[j] = samples[i - filter_window/2 + j];
            }
            filtered_samples[i] = median_filter(window, filter_window);
        }
    }
    
    // 检测物体经过
    int object_count = 0;
    uint32_t total_object_time = 0;
    bool object_detected = false;
    uint32_t object_start_time = 0;
    
    for (int i = 0; i < sample_count; i++) {
        if (filtered_samples[i] >= 0 && filtered_samples[i] < threshold) {
            // 检测到物体
            if (!object_detected) {
                object_detected = true;
                object_start_time = timestamps[i];
                ESP_LOGI(TAG, "Object detected at %.2f cm", filtered_samples[i]);
            }
        } else {
            // 物体离开
            if (object_detected) {
                object_detected = false;
                uint32_t object_end_time = timestamps[i];
                uint32_t object_duration = object_end_time - object_start_time;
                total_object_time += object_duration;
                object_count++;
                ESP_LOGI(TAG, "Object left, duration: %lu ms", (unsigned long)object_duration);
            }
        }
    }
    
    // 计算频率（物体/秒）
    float sample_time_sec = sample_time / 1000.0;
    *frequency = object_count / sample_time_sec;
    
    // 计算密度
    if (*frequency < 0.1) {
        *density = 0; // 低密度
    } else if (*frequency < 0.5) {
        *density = 1; // 中密度
    } else {
        *density = 2; // 高密度
    }
    
    ESP_LOGI(TAG, "Motion detection complete: %d objects, frequency: %.2f objects/sec, density: %d", 
             object_count, *frequency, *density);
    
    return ESP_OK;
}

/**
 * @brief 计算数组的平均值
 * @param data 数据数组
 * @param size 数组大小
 * @return float 平均值
 */
static float calculate_average(float *data, int size)
{
    float sum = 0.0;
    int valid_count = 0;
    
    for (int i = 0; i < size; i++) {
        if (data[i] >= 0) {
            sum += data[i];
            valid_count++;
        }
    }
    
    return valid_count > 0 ? sum / valid_count : -1.0;
}

/**
 * @brief 计算数组的方差
 * @param data 数据数组
 * @param size 数组大小
 * @return float 方差
 */
static float calculate_variance(float *data, int size)
{
    float avg = calculate_average(data, size);
    if (avg < 0) {
        return -1.0;
    }
    
    float sum_sq = 0.0;
    int valid_count = 0;
    
    for (int i = 0; i < size; i++) {
        if (data[i] >= 0) {
            sum_sq += pow(data[i] - avg, 2);
            valid_count++;
        }
    }
    
    return valid_count > 1 ? sum_sq / (valid_count - 1) : 0.0;
}

/**
 * @brief 添加数据到覆盖式缓存
 * @param distance 距离值
 */
void ultrasonic_add_to_cache(float distance)
{
    if (cache_mutex == NULL) {
        return;
    }
    
    if (xSemaphoreTake(cache_mutex, portMAX_DELAY) == pdTRUE) {
        // 覆盖式存储
        distance_cache[cache_index] = distance;
        cache_index = (cache_index + 1) % CACHE_SIZE;
        if (cache_count < CACHE_SIZE) {
            cache_count++;
        }
        xSemaphoreGive(cache_mutex);
    }
}

/**
 * @brief 获取缓存中的数据
 * @param data 输出缓冲区
 * @param size 缓冲区大小
 * @return int 实际数据数量
 */
int ultrasonic_get_cache(float *data, int size)
{
    if (cache_mutex == NULL || data == NULL) {
        return 0;
    }
    
    int count = 0;
    if (xSemaphoreTake(cache_mutex, portMAX_DELAY) == pdTRUE) {
        count = cache_count;
        if (count > size) {
            count = size;
        }
        
        // 从缓存中复制数据
        for (int i = 0; i < count; i++) {
            int index = (cache_index - count + i + CACHE_SIZE) % CACHE_SIZE;
            data[i] = distance_cache[index];
        }
        
        xSemaphoreGive(cache_mutex);
    }
    
    return count;
}

/**
 * @brief 采样任务函数
 * @param pvParameters 任务参数
 */
void ultrasonic_sampling_task(void *pvParameters)
{
    uint32_t start_time = esp_timer_get_time();
    uint32_t duration = 30000000; // 30秒（微秒）
    
    ESP_LOGI(TAG, "Starting 30-second sampling task");
    
    while (esp_timer_get_time() - start_time < duration) {
        // 快速测量距离
        float distance = ultrasonic_measure_distance_fast();
        
        // 添加到缓存
        ultrasonic_add_to_cache(distance);
        
        // 发送到队列（非阻塞）
        xQueueSend(distance_queue, &distance, 0);
        
        // 轻量延时（50ms）
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    ESP_LOGI(TAG, "30-second sampling completed");
    
    // 发送采样完成事件
    set_event(EVENT_SAMPLING_DONE);
    
    sampling_task_handle = NULL; // 清除任务句柄
    vTaskDelete(NULL);
}

/**
 * @brief 启动30秒采样
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 */
esp_err_t ultrasonic_start_30s_sampling(void)
{
    // 先检查是否已有任务在运行
    if (sampling_task_handle != NULL) {
        ESP_LOGW(TAG, "Sampling task already running");
        return ESP_OK;
    }
    
    // 创建采样任务
    BaseType_t ret = xTaskCreate(ultrasonic_sampling_task, "ultrasonic_sampling", 2048, NULL, 5, &sampling_task_handle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create sampling task");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "30-second sampling task created");
    return ESP_OK;
}

/**
 * @brief 获取采样结果
 * @param density 密度指针（返回值：0-低，1-中，2-高）
 * @param motion_change 运动变化指针（返回值：0-小，1-中，2-大）
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 */
esp_err_t ultrasonic_get_sampling_result(int *density, float *motion_change)
{
    if (density == NULL || motion_change == NULL) {
        return ESP_FAIL;
    }
    
    // 获取缓存数据
    float cache_data[CACHE_SIZE];
    int count = ultrasonic_get_cache(cache_data, CACHE_SIZE);
    
    if (count < 5) {
        ESP_LOGE(TAG, "Insufficient data in cache");
        return ESP_FAIL;
    }
    
    // 计算方差
    float variance = calculate_variance(cache_data, count);
    if (variance < 0) {
        ESP_LOGE(TAG, "Failed to calculate variance");
        return ESP_FAIL;
    }
    
    // 计算平均距离
    float avg_distance = calculate_average(cache_data, count);
    if (avg_distance < 0) {
        ESP_LOGE(TAG, "Failed to calculate average distance");
        return ESP_FAIL;
    }
    
    // 根据方差和平均距离计算密度
    // 密度 = f(平均距离, 方差)
    float density_score = 0.0;
    if (avg_distance < 100) { // 近距离
        density_score = 1.0 / (avg_distance / 100.0) * (variance / 100.0 + 1);
    } else { // 远距离
        density_score = 100.0 / avg_distance * (variance / 100.0 + 1);
    }
    
    // 密度等级
    if (density_score < 0.3) {
        *density = 0; // 低密度
    } else if (density_score < 0.7) {
        *density = 1; // 中密度
    } else {
        *density = 2; // 高密度
    }
    
    // 运动变化（直接使用方差）
    *motion_change = variance;
    
    ESP_LOGI(TAG, "Sampling result - Average distance: %.2f cm, Variance: %.2f, Density: %d, Motion change: %.2f", 
             avg_distance, variance, *density, *motion_change);
    
    return ESP_OK;
}

/**
 * @brief 停止采样任务
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 */
esp_err_t ultrasonic_stop_sampling(void)
{
    if (sampling_task_handle != NULL) {
        ESP_LOGI(TAG, "Stopping sampling task");
        vTaskDelete(sampling_task_handle);
        sampling_task_handle = NULL;
        return ESP_OK;
    }
    return ESP_OK;
}

/**
 * @brief 清理超声波模块资源
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 */
esp_err_t ultrasonic_deinit(void)
{
    // 停止采样任务
    ultrasonic_stop_sampling();
    
    // 释放互斥锁
    if (cache_mutex != NULL) {
        vSemaphoreDelete(cache_mutex);
        cache_mutex = NULL;
    }
    
    // 释放队列
    if (distance_queue != NULL) {
        vQueueDelete(distance_queue);
        distance_queue = NULL;
    }
    
    ESP_LOGI(TAG, "Ultrasonic sensor deinitialized");
    return ESP_OK;
}

/**
 * @brief 获取距离队列
 * @return QueueHandle_t - 距离队列句柄
 */
QueueHandle_t ultrasonic_get_distance_queue(void)
{
    return distance_queue;
}