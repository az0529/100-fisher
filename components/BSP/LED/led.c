#include "led.h"

void led_init(void)
{
    gpio_config_t gpio_init_struct = {0};
    gpio_init_struct.intr_type = GPIO_INTR_DISABLE;
    gpio_init_struct.mode = GPIO_MODE_INPUT_OUTPUT;
    gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_init_struct.pin_bit_mask = (1ULL << LED_GPIO_PIN);
    
    gpio_config(&gpio_init_struct);

    LED(1);
}

void led_on(void)
{
    LED(0);
}

void led_off(void)
{
    LED(1);
}

void led_test(void)
{
    for (int i = 0; i < 5; i++) {
        led_on();
        vTaskDelay(pdMS_TO_TICKS(500));
        led_off();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}