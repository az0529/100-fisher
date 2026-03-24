#include "ledc.h"

/**
 * @brief 计算 LEDC 占空比计数值
 * @param duty 占空比 (0-100%)
 * @param m 基数 (通常为 2)
 * @param n 指数 (占空比分辨率)
 * @return 计算后的占空比计数值
 * @description 根据占空比分辨率计算实际的计数值，公式：(m^n * duty) / 100
 */
uint32_t ledc_duty_pow(uint32_t duty, uint8_t m, uint8_t n)
{
	uint32_t result = 1;
	
	/* 计算 m 的 n 次方 */
	while (n--)
	{
		result *= m;
	}

	/* 计算实际占空比计数值 */
	return (result * duty) / 100;
	
}

/**
 * @brief LEDC PWM 初始化函数
 * @param ledc_config LEDC 配置结构体指针
 *        - duty_resolution: 占空比分辨率 (bit)
 *        - timer_num: 定时器编号
 *        - freq_hz: PWM 频率 (Hz)
 *        - clk_cfg: 时钟源配置
 *        - channel: PWM 输出通道
 *        - gpio_num: 输出 GPIO 引脚
 *        - duty: 初始占空比 (0-100%)
 * @return 无
 * @description 初始化 LEDC 定时器和通道，配置 PWM 输出参数
 */
void ledc_init(ledc_config_t *ledc_config)
{
	/* 计算实际占空比：将百分比转换为对应分辨率的计数值 */
	ledc_config->duty = ledc_duty_pow(ledc_config->duty, 2, ledc_config->duty_resolution);

	/* 配置 LEDC 定时器参数 */
	ledc_timer_config_t ledc_timer = {
		.speed_mode 	  = 	LEDC_LOW_SPEED_MODE,           /* 工作模式：低速模式 */
		.duty_resolution  = 	ledc_config->duty_resolution,  /* 占空比分辨率 */
		.timer_num        = 	ledc_config->timer_num,         /* 选择定时器 */
        .freq_hz          = 	ledc_config->freq_hz,           /* 设置 PWM 频率 */
        .clk_cfg          = 	ledc_config->clk_cfg            /* 设置时钟源 */
	};
	/* 应用定时器配置 */
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    
	/* 配置 LEDC 通道参数 */
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = 	LEDC_LOW_SPEED_MODE,      /* 工作模式：低速模式 */
        .channel        = 	ledc_config->channel,     /* PWM 输出通道 */
        .timer_sel      = 	ledc_config->timer_num,   /* 选择使用的定时器 */
        .intr_type      =	LEDC_INTR_DISABLE,        /* 禁用 LEDC 中断 */
        .gpio_num       = 	ledc_config->gpio_num,    /* 输出 GPIO 引脚 */
        .duty           = 	ledc_config->duty,        /* 占空比 */
        .hpoint         = 	0                         /* 起始点设置 */
    };
	/* 应用通道配置 */
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

/**
 * @brief 设置 LEDC PWM 占空比
 * @param ledc_config LEDC 配置结构体指针
 * @param duty 占空比值 (0-100%)
 * @return 无
 * @description 更新指定通道的 PWM 占空比
 */
void ledc_pwm_set_duty(ledc_config_t *ledc_config, uint16_t duty)
{
    /* 计算实际占空比：将百分比转换为对应分辨率的计数值 */
    ledc_config->duty = ledc_duty_pow(duty, 2, ledc_config->duty_resolution);
    
    /* 设置占空比 */
    ledc_set_duty(LEDC_LOW_SPEED_MODE, ledc_config->channel, ledc_config->duty);
    
    /* 应用占空比更新 */
    ledc_update_duty(LEDC_LOW_SPEED_MODE, ledc_config->channel);
}