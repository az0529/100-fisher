#include "adc.h"

adc_oneshot_unit_handle_t adc_handle = NULL;    /* ADC句柄 */


/**
 * @brief 初始化ADC模块
 * @return 无
 * @note 该函数完成以下初始化工作：
 *       1. 配置ADC单元1的基本参数
 *       2. 初始化ADC为单次转换模式
 *       3. 配置ADC通道的衰减和分辨率
 */
void adc_init(void)
{
    /* ADC单元初始化配置结构体 */
    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id  = ADC_UNIT_1,                 /* ADC单元:ADC1 */
        .ulp_mode = ADC_ULP_MODE_DISABLE,       /* 禁用ULP模式，不支持ADC在ULP模式下工作 */
    };
    /* 初始化ADC为单次转换模式，获取ADC句柄 */
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_config, &adc_handle));

    /* ADC通道配置结构体 */
    adc_oneshot_chan_cfg_t config = {
        .atten    = ADC_ATTEN_DB_12,            /* ADC衰减：12dB，可测量更大范围的电压 */
        .bitwidth = ADC_BITWIDTH_12,            /* ADC分辨率：12位，可获得4096级精度 */
    };
    /* 配置指定的ADC通道 */
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHAN, &config));
}

#define LOST_VAL    1

/**
 * @brief 获取ADC通道采样结果的平均值
 * @param ch: ADC通道 通道号, 0~9
 * @param times: 采样次数
 * @return 采样结果的平均值
 * @note 该函数通过以下步骤计算平均值：
 *       1. 分配内存存储采样数据
 *       2. 进行多次ADC采样
 *       3. 对采样数据进行升序排序
 *       4. 去掉两端的极端值（LOST_VAL个）
 *       5. 计算剩余数据的平均值
 */
uint32_t adc_get_result_average(adc_channel_t ch, uint32_t times)
{
    uint32_t sum = 0;         /* 用于累加有效采样值 */
    uint16_t temp_val = 0;     /* 用于排序时的临时变量 */

    /* 申请存放ADC原始数据的buffer */
    int *rawdata = heap_caps_malloc(times * sizeof(int), MALLOC_CAP_INTERNAL);     
    if (NULL == rawdata)
    {
        ESP_LOGE("adc", "Memory for adc is not enough");
        return 0; /* 内存分配失败时返回0 */
    }

    for (uint32_t t = 0; t < times; t++)                /* 多次ADC采样 */
    {
        adc_oneshot_read(adc_handle, ch, &rawdata[t]);  /* 读取原始数据到buffer */
        vTaskDelay(pdMS_TO_TICKS(5));                   /* 采样间隔5ms，避免采样过于密集 */
    }

    for (uint16_t i = 0; i < times - 1; i++)            /* 对数据进行冒泡排序 */
    {
        for (uint16_t j = i + 1; j < times; j++)
        {
            if (rawdata[i] > rawdata[j])                /* 升序排列 */
            {
                temp_val   = rawdata[i];
                rawdata[i] = rawdata[j];
                rawdata[j] = temp_val;
            }
        }
    }

    for (uint32_t i = LOST_VAL; i < times - LOST_VAL; i++)      /* 去掉两端的丢弃值 */
    {
        sum += rawdata[i];                                      /* 累加去掉丢弃值以后的数据 */
    }

    free(rawdata);                                              /* 释放内存 */
    return sum / (times - 2 * LOST_VAL);                        /* 返回平均值 */
}