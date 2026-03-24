#ifndef __ENCODER_EXIT_H__
#define __ENCODER_EXIT_H__

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define ENCODER_A_PIN GPIO_NUM_14  // A 相连接的 GPIO 引脚
#define ENCODER_B_PIN GPIO_NUM_12  // B 相连接的 GPIO 引脚
#define ENCODER_C_PIN GPIO_NUM_13  // C 相

void encoder_gpio_init();

// 外部变量声明
extern volatile uint8_t current_mode;    // 当前模式编号（1-10循环）
extern volatile uint8_t button_pressed_flags;  // 按钮按下标志位

// 外部函数声明
extern void IRAM_ATTR encoder_isr_handler(void* arg);
extern void IRAM_ATTR button_isr_handler(void* arg);

#endif