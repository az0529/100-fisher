#include "oled.h"
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "encoder_exit.h"

extern volatile uint8_t current_mode;
extern volatile uint8_t button_pressed_flags;

void app_main(void)
{
    i2c_bus_init();
    OLED_Init();
    encoder_gpio_init();
    led_init();
    OLED_Clear();

    while (1)
    {     
       OLED_ShowString(0,0,"Current Mode:");
       OLED_ShowNum(96,0,current_mode,2);
       vTaskDelay(100);
    }


    
    

    
}