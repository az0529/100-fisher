#include "buzzer.h"

// 全局蜂鸣器配置变量
static buzzer_config_t buzzer_config = {
    .gpio_num = BUZZER_GPIO_NUM,
    .channel = BUZZER_CHANNEL,
    .timer_num = BUZZER_TIMER,
    .freq_hz = BUZZER_FREQ,
    .duty_resolution = BUZZER_RESOLUTION,
    .volume = BUZZER_VOLUME_MEDIUM
};

/**
 * @brief 蜂鸣器初始化函数
 * @return 无
 * @description 初始化蜂鸣器的LEDC定时器和通道
 */
void buzzer_init(void)
{
    ledc_config_t ledc_config = {
        .clk_cfg = LEDC_AUTO_CLK,                     /* 自动选择时钟源 */
        .timer_num = buzzer_config.timer_num,         /* 选择定时器 */
        .freq_hz = buzzer_config.freq_hz,             /* 设置 PWM 频率 */
        .duty_resolution = buzzer_config.duty_resolution,  /* 占空比分辨率 */
        .channel = buzzer_config.channel,             /* PWM 输出通道 */
        .duty = buzzer_config.volume,                /* 初始占空比（音量） */
        .gpio_num = buzzer_config.gpio_num           /* 输出 GPIO 引脚 */
    };
    
    // 使用ledc_init函数初始化
    ledc_init(&ledc_config);
}

/**
 * @brief 设置蜂鸣器音量
 * @param volume 音量 (0-100%)
 * @return 无
 */
void buzzer_set_volume(uint8_t volume)
{
    if (volume > 100) volume = 100;
    
    buzzer_config.volume = volume;
    
    ledc_config_t ledc_config = {
        .channel = buzzer_config.channel,             /* PWM 输出通道 */
        .duty_resolution = buzzer_config.duty_resolution  /* 占空比分辨率 */
    };
    
    ledc_pwm_set_duty(&ledc_config, volume);
}

/**
 * @brief 设置蜂鸣器频率 仅在无源蜂鸣器适用
 * @param freq 频率 (Hz)
 * @return 无
 * @description 更新蜂鸣器的频率
 */
void buzzer_set_freq(uint32_t freq)
{
    /* 更新频率值 */
    buzzer_config.freq_hz = freq;
    
    /* 重新配置定时器 */
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,           /* 工作模式：低速模式 */
        .duty_resolution  = buzzer_config.duty_resolution,  /* 占空比分辨率 */
        .timer_num        = buzzer_config.timer_num,         /* 选择定时器 */
        .freq_hz          = freq,           /* 设置 PWM 频率 */
        .clk_cfg          = LEDC_AUTO_CLK            /* 自动选择时钟源 */
    };
    /* 应用定时器配置 */
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
}

/**
 * @brief 打开蜂鸣器
 * @return 无
 * @description 打开蜂鸣器（设置为当前音量）
 */
void buzzer_on(void)
{
    // 转换为ledc_config_t结构体
    ledc_config_t ledc_config = {
        .channel = buzzer_config.channel,             /* PWM 输出通道 */
        .duty_resolution = buzzer_config.duty_resolution  /* 占空比分辨率 */
    };
    
    // 使用ledc_pwm_set_duty函数设置占空比为当前音量
    ledc_pwm_set_duty(&ledc_config, buzzer_config.volume);
}

/**
 * @brief 关闭蜂鸣器
 * @return 无
 * @description 关闭蜂鸣器（设置占空比为0）
 */
void buzzer_off(void)
{
    // 转换为ledc_config_t结构体
    ledc_config_t ledc_config = {
        .channel = buzzer_config.channel,             /* PWM 输出通道 */
        .duty_resolution = buzzer_config.duty_resolution  /* 占空比分辨率 */
    };
    
    // 使用ledc_pwm_set_duty函数设置占空比为0
    ledc_pwm_set_duty(&ledc_config, 0);
}

/**
 * @brief 蜂鸣器响指定时间
 * @param duration_ms 持续时间 (毫秒)
 * @return 无
 * @description 打开蜂鸣器，持续指定时间后关闭
 */
void buzzer_beep(uint32_t duration_ms)
{
    /* 打开蜂鸣器 */
    buzzer_on();
    
    /* 延时指定时间 */
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    
    /* 关闭蜂鸣器 */
    buzzer_off();
}