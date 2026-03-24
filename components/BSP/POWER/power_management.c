#include "power_management.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "oled.h"
#include "buzzer.h"
#include "ds18b20.h"
#include "ultrasonic.h"
#include "led.h"
#include "encoder_exit.h"

static const char *TAG = "power_management";

/**
 * @brief 初始化低功耗管理
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 */
esp_err_t power_management_init(void)
{
    ESP_LOGI(TAG, "Initializing power management");
    
    // 配置唤醒源
    // 这里可以根据需要配置不同的唤醒源
    
    return ESP_OK;
}

/**
 * @brief 进入正常模式
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 * @description 正常模式下，所有外设都处于开启状态，系统正常运行
 */
esp_err_t enter_normal_mode(void)
{
    ESP_LOGI(TAG, "Entering normal mode");
    
    // 初始化并开启所有外设
    OLED_Init(); // 初始化OLED
    led_init(); // 初始化LED
    buzzer_init(); // 初始化蜂鸣器
    ultrasonic_init(); // 初始化超声波模块
    
    // 显示正常模式提示
    OLED_Clear();
    OLED_ShowString(30, 2, "Normal Mode", 8);
    OLED_ShowString(10, 4, "All systems active", 8);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 启用旋转编码器的所有中断
    encoder_gpio_init();
    
    ESP_LOGI(TAG, "Entered normal mode");
    
    return ESP_OK;
}

/**
 * @brief 进入轻量级睡眠模式
 * @param sleep_time_ms 睡眠时间（毫秒）
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 * @description 轻量级睡眠模式下，关闭与人交互相关的外设和传感器，禁用旋转编码器的左右旋转功能，只有中间按钮可以退出此模式，同时关闭WiFi以降低功耗
 */
esp_err_t enter_light_sleep(uint32_t sleep_time_ms)
{
    ESP_LOGI(TAG, "Entering light sleep mode for %d ms", sleep_time_ms);
    
    // 显示睡眠提示
    OLED_Clear(); // 清空OLED显示
    OLED_ShowString(30, 2, "Sleep Mode", 8);
    OLED_ShowString(10, 4, "Press to wake up", 8);
    vTaskDelay(pdMS_TO_TICKS(500)); // 显示提示信息
    
    // 关闭与人交互相关的外设
    OLED_PowerDown(); // 关闭OLED电源
    
    // 关闭蜂鸣器
    buzzer_off();
    
    // 关闭LED
    led_off();
    
    // 停止超声波采样
    ultrasonic_stop_sampling();
    
    // 关闭WiFi
    esp_wifi_stop();
    esp_wifi_deinit();
    ESP_LOGI(TAG, "WiFi disabled");
    
    // 禁用旋转编码器的A相和B相中断（左右旋转）
    gpio_isr_handler_remove(ENCODER_A_PIN);
    gpio_isr_handler_remove(ENCODER_B_PIN);
    
    // 等待按钮按下或超时
    uint32_t start_time = esp_timer_get_time() / 1000; // 转换为毫秒
    while (esp_timer_get_time() / 1000 - start_time < sleep_time_ms) {
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            break; // 按钮按下，退出睡眠模式
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 短延时，避免占用CPU
    }
    
    // 唤醒后重新初始化外设
    OLED_PowerOn(); // 开启OLED电源
    OLED_Init(); // 重新初始化OLED
    led_init(); // 重新初始化LED
    buzzer_init(); // 重新初始化蜂鸣器
    ultrasonic_init(); // 重新初始化超声波模块
    
    // 重新启用旋转编码器的A相和B相中断
    gpio_isr_handler_add(ENCODER_A_PIN, encoder_isr_handler, NULL);
    gpio_isr_handler_add(ENCODER_B_PIN, encoder_isr_handler, NULL);
    
    ESP_LOGI(TAG, "Exited light sleep mode");
    
    return ESP_OK;
}

/**
 * @brief 进入深度睡眠模式
 * @param sleep_time_ms 睡眠时间（毫秒）
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 * @description 深度睡眠模式下，关闭所有外设，CPU和大部分外设断电，只有RTC和部分GPIO保持供电，功耗最低，同时关闭WiFi和蓝牙以进一步降低功耗
 */
esp_err_t enter_deep_sleep(uint32_t sleep_time_ms)
{
    ESP_LOGI(TAG, "Entering deep sleep for %d ms", sleep_time_ms);
    
    // 显示深度睡眠提示
    OLED_Clear();
    OLED_ShowString(20, 2, "Deep Sleep Mode", 8);
    OLED_ShowString(10, 4, "System will restart", 8);
    OLED_ShowString(10, 5, "after sleep", 8);
    vTaskDelay(pdMS_TO_TICKS(1000)); // 显示提示信息
    
    // 关闭所有外设
    OLED_PowerDown(); // 关闭OLED电源
    buzzer_off(); // 关闭蜂鸣器
    led_off(); // 关闭LED
    
    // 停止超声波采样
    ultrasonic_stop_sampling();
    ultrasonic_deinit();
    
    // 关闭WiFi
    esp_wifi_stop();
    esp_wifi_deinit();
    ESP_LOGI(TAG, "WiFi disabled");
    
    /* 关闭蓝牙
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    ESP_LOGI(TAG, "Bluetooth disabled");*/
    
    // 配置唤醒源
    // 1. 定时器唤醒
    esp_sleep_enable_timer_wakeup(sleep_time_ms * 1000); // 转换为微秒
    
    // 2. GPIO唤醒（旋转编码器的中间按钮）
    rtc_gpio_deinit(ENCODER_C_PIN);
    rtc_gpio_init(ENCODER_C_PIN);
    rtc_gpio_set_direction(ENCODER_C_PIN, RTC_GPIO_MODE_INPUT_ONLY);
    esp_sleep_enable_ext0_wakeup(ENCODER_C_PIN, 0); // 低电平唤醒
    
    // 进入深度睡眠
    ESP_LOGI(TAG, "Deep sleep configured, entering sleep now");
    esp_deep_sleep_start();
    
    // 注意：深度睡眠后会重启，所以这里的代码不会执行
    return ESP_OK;
}


/**
 * @brief 进入休眠模式
 * @param sleep_time_ms 睡眠时间（毫秒）
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 * @description 休眠模式下，关闭所有外设，除了RTC和部分GPIO外，所有外设都断电，功耗最低，适合长时间睡眠
 */
esp_err_t enter_hibernation(uint32_t sleep_time_ms)
{
    ESP_LOGI(TAG, "Entering hibernation for %d ms", sleep_time_ms);
    
    // 显示休眠提示
    OLED_Clear();
    OLED_ShowString(30, 2, "Hibernation", 8);
    OLED_ShowString(10, 4, "Long term sleep", 8);
    OLED_ShowString(10, 5, "Lowest power", 8);
    vTaskDelay(pdMS_TO_TICKS(1000)); // 显示提示信息
    
    // 关闭所有外设
    OLED_PowerDown(); // 关闭OLED电源
    buzzer_off(); // 关闭蜂鸣器
    led_off(); // 关闭LED
    
    // 停止超声波采样
    ultrasonic_stop_sampling();
    ultrasonic_deinit();
    
    // 关闭WiFi
    esp_wifi_stop();
    esp_wifi_deinit();
    ESP_LOGI(TAG, "WiFi disabled");
    
    /*关闭蓝牙
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    ESP_LOGI(TAG, "Bluetooth disabled");*/ 
    
    // 配置唤醒源
    // 1. 定时器唤醒
    esp_sleep_enable_timer_wakeup(sleep_time_ms * 1000); // 转换为微秒
    
    // 2. GPIO唤醒（旋转编码器的中间按钮）
    rtc_gpio_deinit(ENCODER_C_PIN);
    rtc_gpio_init(ENCODER_C_PIN);
    rtc_gpio_set_direction(ENCODER_C_PIN, RTC_GPIO_MODE_INPUT_ONLY);
    esp_sleep_enable_ext0_wakeup(ENCODER_C_PIN, 0); // 低电平唤醒
    
    // 进入深度睡眠（ESP32的深度睡眠已经是最低功耗模式）
    ESP_LOGI(TAG, "Hibernation configured, entering sleep now");
    esp_deep_sleep_start();
    
    // 注意：休眠模式后会重启，所以这里的代码不会执行
    return ESP_OK;
}


/**
 * @brief 配置GPIO唤醒源
 * @param gpio_num GPIO编号
 * @param level 唤醒电平（1为高电平，0为低电平）
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 */
esp_err_t config_gpio_wakeup(int gpio_num, int level)
{
    ESP_LOGI(TAG, "Configuring GPIO %d as wakeup source with level %d", gpio_num, level);
    
    // 配置GPIO为RTC IO
    rtc_gpio_deinit(gpio_num);
    rtc_gpio_init(gpio_num);
    rtc_gpio_set_direction(gpio_num, RTC_GPIO_MODE_INPUT_ONLY);
    
    // 配置GPIO唤醒源
    if (level) {
        esp_sleep_enable_ext0_wakeup(gpio_num, 1);
    } else {
        esp_sleep_enable_ext0_wakeup(gpio_num, 0);
    }
    
    return ESP_OK;
}


/**
 * @brief 获取当前功耗模式
 * @return const char* - 当前功耗模式的名称
 */
const char* get_current_power_mode(void)
{
    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            return "Active";
        case ESP_SLEEP_WAKEUP_TIMER:
            return "Woke up from timer";
        case ESP_SLEEP_WAKEUP_EXT0:
            return "Woke up from GPIO";
        case ESP_SLEEP_WAKEUP_EXT1:
            return "Woke up from GPIO array";
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            return "Woke up from touchpad";
        case ESP_SLEEP_WAKEUP_ULP:
            return "Woke up from ULP";
        default:
            return "Unknown";
    }
}