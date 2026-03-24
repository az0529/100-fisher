#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "driver/gpio.h"
#include "ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// 蜂鸣器配置
#define BUZZER_GPIO_NUM    GPIO_NUM_3     // 蜂鸣器引脚
#define BUZZER_CHANNEL     LEDC_CHANNEL_2 // 蜂鸣器通道
#define BUZZER_TIMER       LEDC_TIMER_1   // 蜂鸣器定时器
#define BUZZER_FREQ        2000           // 蜂鸣器默认频率（Hz）
#define BUZZER_RESOLUTION  LEDC_TIMER_8_BIT // 蜂鸣器分辨率

// 音量级别定义
#define BUZZER_VOLUME_LOW    20  // 低音量（20%占空比）
#define BUZZER_VOLUME_MEDIUM 50  // 中等音量（50%占空比）
#define BUZZER_VOLUME_HIGH   80  // 高音量（80%占空比）

//音高级别定义 仅在无源蜂鸣器适用
#define BUZZER_FREQ_do  262 // C4
#define BUZZER_FREQ_re  294 // D4
#define BUZZER_FREQ_mi  330 // E4
#define BUZZER_FREQ_fo  349 // F4
#define BUZZER_FREQ_so  392 // G4
#define BUZZER_FREQ_la  440 // A4
#define BUZZER_FREQ_si  494 // B4

typedef struct {
    gpio_num_t gpio_num;             // 蜂鸣器连接的GPIO引脚
    ledc_channel_t channel;          // LEDC通道
    ledc_timer_t timer_num;          // LEDC定时器
    uint32_t freq_hz;               // 蜂鸣器频率
    ledc_timer_bit_t duty_resolution; // 占空比分辨率
    uint8_t volume;                 // 音量（占空比百分比）
} buzzer_config_t;

// 蜂鸣器函数
void buzzer_init(void);
void buzzer_set_volume(uint8_t volume);
void buzzer_set_freq(uint32_t freq);
void buzzer_on(void);
void buzzer_off(void);
void buzzer_beep(uint32_t duration_ms);

#endif