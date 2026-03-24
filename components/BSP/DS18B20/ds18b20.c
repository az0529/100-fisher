#include "ds18b20.h"

/**
 * @brief DS18B20初始化函数
 * @return 0：初始化成功，1：初始化失败
 * @description 按照1-wire协议标准初始化DS18B20温度传感器，检测传感器是否存在
 */
unsigned char DS18B20_Init(void)
{
    unsigned char dat;
    
    // 设置GPIO为输出模式
    gpio_set_direction(DS18B20_GPIO_NUM, GPIO_MODE_OUTPUT);
    
    // 恢复时间：>1us
    gpio_set_level(DS18B20_GPIO_NUM, 1);
    ets_delay_us(2);
    
    gpio_set_level(DS18B20_GPIO_NUM, 0);
    ets_delay_us(500);
    
    gpio_set_level(DS18B20_GPIO_NUM, 1);
    ets_delay_us(60);
    
    gpio_set_direction(DS18B20_GPIO_NUM, GPIO_MODE_INPUT);
    
    dat = gpio_get_level(DS18B20_GPIO_NUM);
    
    ets_delay_us(500);
    
    ets_delay_us(500); 
    
    gpio_set_direction(DS18B20_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_GPIO_NUM, 1);
    
    return dat;
}

/**
 * @brief 读一位数据
 * @return 读取的数据位
 * @description 按照1-wire标准读取一位数据
 */
unsigned char DS18B20_ReadOneBit(void)
{
    unsigned char dat = 0;
    
    gpio_set_direction(DS18B20_GPIO_NUM, GPIO_MODE_OUTPUT);
    
    gpio_set_level(DS18B20_GPIO_NUM, 0);
    ets_delay_us(2); 
    
    gpio_set_level(DS18B20_GPIO_NUM, 1);
    ets_delay_us(9);
    
    gpio_set_direction(DS18B20_GPIO_NUM, GPIO_MODE_INPUT);
    
    dat = gpio_get_level(DS18B20_GPIO_NUM);
    
    ets_delay_us(70);
    
    gpio_set_direction(DS18B20_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_GPIO_NUM, 1);
    
    ets_delay_us(2);
    
    return dat;
}

/**
 * @brief 读一个字节数据
 * @return 读取的字节数据
 * @description 按照1-wire标准读取一个字节数据
 */
unsigned char DS18B20_ReadOneChar(void)
{
    unsigned char i = 0;
    unsigned char dat = 0;
    
    for (i = 8; i > 0; i--) {
        dat >>= 1;
        if (DS18B20_ReadOneBit()) {
            dat |= 0x80; 
        }
    }
    
    gpio_set_direction(DS18B20_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_GPIO_NUM, 1);
    
    return dat;
}

/**
 * @brief 写一位数据
 * @param dat 要写入的数据位
 * @return 无
 * @description 按照1-wire标准写入一位数据
 */
void DS18B20_WriteOneBit(unsigned char dat)
{
    gpio_set_direction(DS18B20_GPIO_NUM, GPIO_MODE_OUTPUT);
    
    gpio_set_level(DS18B20_GPIO_NUM, 0);
    ets_delay_us(2);
    
    gpio_set_level(DS18B20_GPIO_NUM, dat);
    ets_delay_us(70); 
    
    gpio_set_level(DS18B20_GPIO_NUM, 1);
    
    ets_delay_us(2);
}

/**
 * @brief 写一个字节数据
 * @param dat 要写入的字节数据
 * @return 无
 * @description 按照1-wire标准写入一个字节数据
 */
void DS18B20_WriteOneChar(unsigned char dat)
{
    unsigned char i = 0;
    
    for (i = 8; i > 0; i--) {
        DS18B20_WriteOneBit(dat & 0x01);
        dat >>= 1;
    }
    
    gpio_set_direction(DS18B20_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_GPIO_NUM, 1);
}

/**
 * @brief 获取温度
 * @return 温度值（摄氏度）
 * @description 从DS18B20获取温度值
 */
float DS18B20_GetTemperature(void)
{
    unsigned char temp_l, temp_h;
    int temp;
    float temperature;
    
    // 初始化DS18B20
    if (DS18B20_Init()) {
        return -127.0; // 初始化失败
    }
    
    DS18B20_WriteOneChar(DS18B20_CMD_SKIP);
    
    DS18B20_WriteOneChar(DS18B20_CMD_CONVERT);
    
    ets_delay_us(800000); 
    
    if (DS18B20_Init()) {
        return -127.0; // 初始化失败
    }
    
    DS18B20_WriteOneChar(DS18B20_CMD_SKIP);
    
    DS18B20_WriteOneChar(DS18B20_CMD_READ);
    
    temp_l = DS18B20_ReadOneChar(); // 低8位
    temp_h = DS18B20_ReadOneChar(); // 高8位
    
        temp = (temp_h << 8) | temp_l;
    
    if (temp < 0) { 
        temperature = (float)(temp ^ 0xffff) + 1;
        temperature = -temperature / 16.0;
    } else { 
        temperature = (float)temp / 16.0;
    }
    
    return temperature;
}