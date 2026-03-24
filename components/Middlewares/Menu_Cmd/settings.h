#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "oled.h"
#include "encoder_exit.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 显示设置菜单选项
void show_settings_menu(uint8_t current_option, uint8_t last_option);

// 显示功耗模式菜单
void show_power_mode_menu(uint8_t current_option, uint8_t last_option);

// 显示夜钓模式菜单
void show_night_fishing_menu(uint8_t current_option, uint8_t is_enabled, uint8_t last_option);

// 显示大风模式菜单
void show_strong_wind_menu(uint8_t current_option, uint8_t is_enabled, uint8_t last_option);

// 全局变量，用于存储设置状态
extern uint8_t power_mode;      // 0: Normal, 1: Light Sleep, 2: Deep Sleep
extern uint8_t night_fishing_enabled;  // 0: Off, 1: On
extern uint8_t strong_wind_enabled;    // 0: Off, 1: On

// 初始化设置
void settings_init(void);

// 执行设置菜单
void execute_settings_menu(void);

#endif