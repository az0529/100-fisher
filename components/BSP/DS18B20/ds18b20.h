#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "esp_err.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"

// DS18B20 GPIO引脚定义
#define DS18B20_GPIO_NUM    GPIO_NUM_21

// DS18B20命令
#define DS18B20_CMD_CONVERT 0x44      // 温度转换命令
#define DS18B20_CMD_READ    0xBE      // 读取温度命令
#define DS18B20_CMD_SKIP    0xCC      // 跳过ROM命令

// 函数声明
unsigned char DS18B20_Init(void);
unsigned char DS18B20_ReadOneBit(void);
unsigned char DS18B20_ReadOneChar(void);
void DS18B20_WriteOneBit(unsigned char dat);
void DS18B20_WriteOneChar(unsigned char dat);
float DS18B20_GetTemperature(void);

#endif