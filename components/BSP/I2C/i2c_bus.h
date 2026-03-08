#ifndef _I2C_BUS_H_
#define _I2C_BUS_H_

#include "driver/i2c.h"
#include "driver/gpio.h"

/* 管脚声明 */
#define I2C_SCL GPIO_NUM_4
#define I2C_SDA GPIO_NUM_5

#define I2C_PORT I2C_NUM_0
#define I2C_FREQ 100000

/* 函数声明 */
void i2c_bus_init(void);

esp_err_t i2c_bus_write(uint8_t dev_addr,uint8_t *data,uint8_t len);

#endif