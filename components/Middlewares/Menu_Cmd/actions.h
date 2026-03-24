#ifndef __ACTIONS_H__
#define __ACTIONS_H__

#include "menu.h"
#include "encoder_exit.h"
#include "oled.h"
#include "led.h"
#include "ds18b20.h"
#include "network.h"
#include "buzzer.h"
#include "ultrasonic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 全局变量，用于存储设置状态
static uint8_t power_mode = 0;      // 0: Normal, 1: Light Sleep, 2: Deep Sleep
static uint8_t night_fishing_enabled = 0;  // 0: Off, 1: On
static uint8_t strong_wind_enabled = 0;    // 0: Off, 1: On

void action1(void);
void action2(void);
void action3(void);
void action4(void);
void action5(void);
void action6(void);
void action7(void);
void action8(void);
void action9(void); 
void action10(void);
void showtime(void);
void if_exit(void);


#endif

