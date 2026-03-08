#ifndef _SSD1306_H_
#define _SSD1306_H_

#include <stdint.h>
#include "i2c_bus.h"

void ssd1306_init(void);

void ssd1306_write_cmd(uint8_t cmd);

void ssd1306_write_data(uint8_t data);

void ssd1306_set_cursor(uint8_t page,uint8_t column);

#endif