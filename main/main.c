#include "oled.h"
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "encoder_exit.h"
#include "menu.h"
#include "buzzer.h"
#include "ds18b20.h"
#include "network.h"
#include "power_management.h"
#include "task_comm.h"
#include "ultrasonic.h"
#include "esp_log.h"

static const char *TAG = "main";

// 任务句柄
static TaskHandle_t xMenuTaskHandle = NULL;
static TaskHandle_t xDataTaskHandle = NULL;
static TaskHandle_t xDisplayTaskHandle = NULL;
static TaskHandle_t xEncoderTaskHandle = NULL;

// 全局数据
system_data_t g_system_data = {
    .temperature = 0.0,
    .distance = 0.0,
    .current_time = 0,
    .current_mode = 0,
    .data_ready = false
};

/**
 * @brief 系统初始化任务
 * @param pvParameters 任务参数
 */
void system_init_task(void *pvParameters)
{
    ESP_LOGI(TAG, "System initialization task started");
    
    // 初始化任务通信机制
    task_comm_init();
    
    // 初始化所有外设
    led_init();
    encoder_gpio_init();
    OLED_Init();
    DS18B20_Init();
    buzzer_init();
    ultrasonic_init();
    
    // 进入正常模式
    enter_normal_mode();
    
    // 显示初始化完成信息
    OLED_Clear();
    OLED_ShowString(15, 2, "System Initialized", 8);
    led_on();
    vTaskDelay(pdMS_TO_TICKS(1000));
    OLED_Clear();
    
    ESP_LOGI(TAG, "System initialization completed");
    
    // 创建其他任务
    xTaskCreate(menu_task, "menu_task", 4096, NULL, 4, &xMenuTaskHandle);
    xTaskCreate(data_collection_task, "data_collection_task", 2048, NULL, 3, &xDataTaskHandle);
    xTaskCreate(display_task, "display_task", 2048, NULL, 2, &xDisplayTaskHandle);
    xTaskCreate(encoder_task, "encoder_task", 1024, NULL, 5, &xEncoderTaskHandle);
    
    // 初始化任务完成，删除自身
    vTaskDelete(NULL);
}

/**
 * @brief 菜单任务
 * @param pvParameters 任务参数
 */
void menu_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Menu task started");
    
    while (1) {
        // 处理菜单逻辑
        menu_update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * @brief 数据采集任务
 * @param pvParameters 任务参数
 */
void data_collection_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Data collection task started");
    
    while (1) {
        // 采集温度数据
        g_system_data.temperature = DS18B20_GetTemperature();
        
        // 采集距离数据
        g_system_data.distance = ultrasonic_measure_distance();
        
        // 采集时间数据
        g_system_data.current_time = get_current_time();
        
        // 标记数据就绪
        g_system_data.data_ready = true;
        
        // 设置数据就绪事件
        set_event(EVENT_DATA_READY);
        
        // 500ms采集一次数据
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief 显示任务
 * @param pvParameters 任务参数
 */
void display_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Display task started");
    
    while (1) {
        // 等待数据就绪事件
        if (wait_for_event(EVENT_DATA_READY, pdMS_TO_TICKS(1000))) {
            // 获取显示互斥锁
            if (xSemaphoreTake(g_display_mutex, portMAX_DELAY) == pdTRUE) {
                // 根据当前模式显示不同内容
                switch (g_system_data.current_mode) {
                    case 0: // 实时监测模式
                        showtime();
                        OLED_ShowString(2, 2, "TEMP:", 8);
                        OLED_ShowFloat(36, 2, g_system_data.temperature, 2, 2, 8);
                        OLED_ShowChar(74, 2, 'C', 8);
                        
                        OLED_ShowString(2, 4, "DIST:", 8);
                        if (g_system_data.distance >= 0) {
                            OLED_ShowFloat(40, 4, g_system_data.distance, 2, 1, 8);
                            OLED_ShowString(74, 4, "cm", 8);
                        } else {
                            OLED_ShowString(32, 4, "ERR", 8);
                        }
                        break;
                    // 其他模式的显示逻辑
                    default:
                        break;
                }
                
                // 释放显示互斥锁
                xSemaphoreGive(g_display_mutex);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief 编码器任务
 * @param pvParameters 任务参数
 */
void encoder_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Encoder task started");
    
    while (1) {
        // 检查编码器输入
        encoder_read();
        
        // 检查按键状态
        if (button_pressed_flags != 0) {
            // 发送编码器按键消息
            send_task_msg(MSG_TYPE_ENCODER_INPUT, button_pressed_flags, 0, NULL);
            button_pressed_flags = 0;
            
            // 设置编码器按下事件
            set_event(EVENT_ENCODER_PRESSED);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 multi-function monitoring system");
    
    // 创建系统初始化任务
    xTaskCreate(system_init_task, "system_init_task", 4096, NULL, 5, NULL);
    
    // 应用主函数退出，由FreeRTOS管理任务
}
