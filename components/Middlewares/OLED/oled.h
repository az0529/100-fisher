#ifndef _OLED_H_
#define _OLED_H_

#include "driver/gpio.h"
#include <stdint.h>
#include "ssd1306.h"
#include "oled_font.h"

void OLED_Init(void);

void OLED_Clear(void);

void OLED_ShowChar(uint8_t x,uint8_t y,char chr);

void OLED_ShowString(uint8_t x,uint8_t y,char *str);

void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len);

#endif