#ifndef __ENCODER_EXIT_H__
#define __ENCODER_EXIT_H__

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ENCODER_A_PIN GPIO_NUM_14  // A 相连接的 GPIO 引脚
#define ENCODER_B_PIN GPIO_NUM_12  // B 相连接的 GPIO 引脚
#define ENCODER_C_PIN GPIO_NUM_13  // C 相

void encoder_gpio_init();


#endif
