#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include "esp_err.h"


esp_err_t power_management_init(void);

/**
 * @brief 进入正常模式
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 * @description 正常模式下，所有外设都处于开启状态，系统正常运行
 */
esp_err_t enter_normal_mode(void);
esp_err_t enter_light_sleep(uint32_t sleep_time_ms);
esp_err_t enter_deep_sleep(uint32_t sleep_time_ms);
esp_err_t enter_hibernation(uint32_t sleep_time_ms);
esp_err_t config_gpio_wakeup(int gpio_num, int level);
esp_err_t config_touch_wakeup(int touch_num, uint16_t threshold);
const char* get_current_power_mode(void);

#endif