#include "i2c_bus.h"



/**
 * @brief 初始化 I2C 总线
 * @note 此函数用于初始化 ESP32 的 I2C 总线，配置为主模式
 * @note 配置参数说明：
 *       - 模式：I2C_MODE_MASTER（主模式）
 *       - SDA 引脚：I2C_SDA（从宏定义获取）
 *       - SCL 引脚：I2C_SCL（从宏定义获取）
 *       - SDA 上拉：启用
 *       - SCL 上拉：启用
 *       - 时钟速度：I2C_FREQ（从宏定义获取）
 * @note 实现步骤：
 *       1. 先删除已安装的 I2C 驱动，防止重复安装
 *       2. 配置 I2C 参数结构体
 *       3. 应用参数配置
 *       4. 安装 I2C 驱动
 *       5. 检查驱动安装结果，失败时打印错误信息
 * @return 无
 */
void i2c_bus_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ
    };
    i2c_param_config(I2C_PORT, &conf);
    esp_err_t ret = i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
    if(ret == ESP_ERR_INVALID_STATE){
        printf("I2C driver already installed\n");
    } else if(ret != ESP_OK){
        printf("I2C driver install failed: %d\n", ret);
    }
    
}

/**
 * @brief 向 I2C 设备写入数据
 * @param dev_addr 设备地址
 * @param data 要写入的数据缓冲区
 * @param len 要写入的数据长度
 * @return esp_err_t 操作结果，ESP_OK 表示成功，其他值表示失败
 * @note 实现原理：调用 ESP32 的 i2c_master_write_to_device 函数向指定设备写入数据
 * @note 超时时间：100ms
 */
esp_err_t i2c_bus_write(uint8_t dev_addr,uint8_t *data,uint8_t len)
{
    return i2c_master_write_to_device(
        I2C_PORT,
        dev_addr,
        data,
        len,
        100/portTICK_PERIOD_MS
    );
}