#ifndef __ADC_H
#define __ADC_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"


/* 管脚声明 */
#define ADC_CHAN    ADC_CHANNEL_7       /* 对应管脚为GPIO8 */

/* 函数声明 */
void adc_init(void);    

uint32_t adc_get_result_average(adc_channel_t ch, uint32_t times); 

#endif
