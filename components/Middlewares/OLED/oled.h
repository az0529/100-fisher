#ifndef _OLED_H_
#define _OLED_H_

#include "driver/gpio.h"
#include <stdint.h>
#include "ssd1306.h"
#include "oled_font.h"

void OLED_Init(void);

void OLED_Clear(void);

void OLED_PowerDown(void);
void OLED_PowerOn(void);
void OLED_ON(void);
void OLED_OFF(void);


void OLED_ShowChar(uint8_t x, uint8_t y, char chr, uint8_t size);
void OLED_ShowString(uint8_t x, uint8_t y, char *str, uint8_t size);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
void OLED_ShowSignedNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, uint8_t size);
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t int_len, uint8_t dec_len, uint8_t size);
void OLED_ShowImage(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *image);
void OLED_ShowChinese(uint8_t x, uint8_t y, const char *ch, uint8_t size);

// 基本图形绘制
void OLED_DrawPoint(uint8_t x, uint8_t y);
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void OLED_DrawCircle(uint8_t x0, uint8_t y0, uint8_t radius);
void OLED_ShowBox(uint8_t x, uint8_t y);
void OLED_ShowCheck(uint8_t x, uint8_t y);

// 擦除函数
void OLED_ClearArea(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void OLED_ClearLine(uint8_t y);
void OLED_ClearColumn(uint8_t x, uint8_t start_y, uint8_t end_y);

#endif