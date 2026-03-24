#ifndef __MENU_H__
#define __MENU_H__

#include "encoder_exit.h"
#include "oled.h"
#include "led.h"
#include "ds18b20.h"
#include "network.h"
#include "buzzer.h"
#include "ultrasonic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "actions.h"

// 系统数据结构
typedef struct {
    float temperature;
    float distance;
    time_t current_time;
    uint8_t current_mode;
    bool data_ready;
} system_data_t;

// 动作函数类型定义
typedef void (*action_func_t)(void);

extern char *menu_list[];

extern volatile uint8_t current_mode;
extern volatile uint8_t button_pressed_flags;

extern system_data_t g_system_data;

static uint32_t timeout = 10000000;

void menu_init();
void menu_update();
void mode_action(uint8_t current_mode);

// 声明action函数
extern void action1(void);
extern void action2(void);
extern void action3(void);
extern void action4(void);
extern void action5(void);
extern void action6(void);
extern void action7(void);
extern void action8(void);
extern void action9(void);
extern void action10(void);

// 动作函数指针数组
extern action_func_t action_functions[];

#endif