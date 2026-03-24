#ifndef __LED_H__
#define __LED_H__

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_GPIO_PIN GPIO_NUM_8

enum GPIO_OUTPUT_STATE
{
    GPIO_RESET,
    GPIO_SET
};

#define  LED(X)  do{X?\
                    gpio_set_level(LED_GPIO_PIN, GPIO_SET):\
                    gpio_set_level(LED_GPIO_PIN, GPIO_RESET);\
                    }while(0)

#define LED_TOGGLE()  do{gpio_set_level(LED_GPIO_PIN, !gpio_get_level(LED_GPIO_PIN));}while(0)

void led_init(void);
void led_on(void);
void led_off(void);
void led_test(void);



#endif
