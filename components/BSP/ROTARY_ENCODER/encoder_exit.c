#include "encoder_exit.h"

volatile uint8_t current_mode = 1;    // 当前模式编号（1-10循环）
volatile uint8_t button_pressed_flags = 0;  // 按钮按下标志位
volatile uint8_t last_A = 0;          // 上一次 A 相状态（用于判断旋转方向）
volatile uint8_t last_B = 0;          // 上一次 B 相状态（用于判断旋转方向）

/**
 * @brief 编码器中断处理函数
 * @details 处理旋转编码器的中断，判断旋转方向并切换模式
 * @param arg 中断参数（未使用）
 * @return 无
 * @note 使用 IRAM_ATTR 属性将函数放在 IRAM 中，提高中断处理速度
 */
void IRAM_ATTR encoder_isr_handler(void* arg)
{
    // 读取当前 A 相和 B 相的状态
    uint8_t A = gpio_get_level(ENCODER_A_PIN);
    uint8_t B = gpio_get_level(ENCODER_B_PIN);

    // 检查 A 相或 B 相状态是否发生变化
    if (A != last_A || B != last_B)
    {
        // 当 A 相变化时，根据 B 相的状态判断方向
        if (A != last_A)
        {
            if (A == B)
            {
                // 顺时针方向，只有当完成完整的一格旋转时才更新模式
                static uint8_t last_state = 0;
                uint8_t current_state = (A << 1) | B;
                
                // 顺时针旋转的完整状态序列：00->10->11->01->00
                if ((last_state == 0x00 && current_state == 0x01) ||
                    (last_state == 0x01 && current_state == 0x03) ||
                    (last_state == 0x03 && current_state == 0x02) ||
                    (last_state == 0x02 && current_state == 0x00))
                {
                    // 完成完整的一格顺时针旋转
                    current_mode++;
                    if (current_mode > 10)
                        {current_mode = 1; }
                }
                last_state = current_state;
            }
            else
            {
                // 逆时针方向，只有当完成完整的一格旋转时才更新模式
                static uint8_t last_state = 0;
                uint8_t current_state = (A << 1) | B;
                
                // 逆时针旋转的完整状态序列：00->01->11->10->00
                if ((last_state == 0x00 && current_state == 0x02) ||
                    (last_state == 0x02 && current_state == 0x03) ||
                    (last_state == 0x03 && current_state == 0x01) ||
                    (last_state == 0x01 && current_state == 0x00))
                {
                    // 完成完整的一格逆时针旋转
                    if (current_mode == 1) 
                        {current_mode = 10;}
                    else 
                        {current_mode--; }
                }
                last_state = current_state;
            }
        }
    }

    // 更新上一次的 A 和 B 状态
    last_A = A;
    last_B = B;
}
void IRAM_ATTR button_isr_handler(void* arg)
{
    button_pressed_flags = 1;
}


/**
 * @brief 旋转编码器GPIO初始化函数
 * @details 配置旋转编码器的A、B相引脚和按钮引脚为输入模式，
 *          启用上拉电阻，配置外部中断并绑定中断处理函数，最后启用全局中断
 * @return 无
 */
void encoder_gpio_init()
{
      gpio_config_t gpio_init_struct_A = {0};
    gpio_init_struct_A.intr_type = GPIO_INTR_ANYEDGE;  // A 相中断触发类型为任意边沿
    gpio_init_struct_A.mode = GPIO_MODE_INPUT;  // 设置为输入模式
    gpio_init_struct_A.pull_up_en = GPIO_PULLUP_ENABLE;  // 启用上拉电阻
    gpio_init_struct_A.pull_down_en = GPIO_PULLDOWN_DISABLE;  // 禁用下拉电阻
    gpio_init_struct_A.pin_bit_mask = (1ULL << ENCODER_A_PIN);  // 设置为 A 相引脚
    gpio_config(&gpio_init_struct_A);  // 配置 A 相引脚

    // 配置 B 相引脚
    gpio_config_t gpio_init_struct_B = {0};
    gpio_init_struct_B.intr_type = GPIO_INTR_ANYEDGE;  // B 相中断触发类型为任意边沿
    gpio_init_struct_B.mode = GPIO_MODE_INPUT;  // 设置为输入模式
    gpio_init_struct_B.pull_up_en = GPIO_PULLUP_ENABLE;  // 启用上拉电阻
    gpio_init_struct_B.pull_down_en = GPIO_PULLDOWN_DISABLE;  // 禁用下拉电阻
    gpio_init_struct_B.pin_bit_mask = (1ULL << ENCODER_B_PIN);  // 设置为 B 相引脚
    gpio_config(&gpio_init_struct_B);  // 配置 B 相引脚

    // 配置中间按钮引脚（C 相）
    gpio_config_t gpio_init_struct_C = {0};
    gpio_init_struct_C.intr_type = GPIO_INTR_POSEDGE;  // 按钮上升沿触发
    gpio_init_struct_C.mode = GPIO_MODE_INPUT;  // 设置为输入模式
    gpio_init_struct_C.pull_up_en = GPIO_PULLUP_ENABLE;  // 启用上拉电阻
    gpio_init_struct_C.pull_down_en = GPIO_PULLDOWN_DISABLE;  // 禁用下拉电阻
    gpio_init_struct_C.pin_bit_mask = (1ULL << ENCODER_C_PIN);  // 设置为 C 相按钮引脚
    gpio_config(&gpio_init_struct_C);  // 配置 C 相按钮引脚

    // 安装中断服务
    gpio_install_isr_service(0);  // 注册 GPIO 中断服务

    // 为 A 相、B 相和 C 相添加中断处理函数
    gpio_isr_handler_add(ENCODER_A_PIN, encoder_isr_handler, NULL);  // 为 A 相添加中断处理函数
    gpio_isr_handler_add(ENCODER_B_PIN, encoder_isr_handler, NULL);  // 为 B 相添加中断处理函数
    gpio_isr_handler_add(ENCODER_C_PIN, button_isr_handler, NULL);   // 为按钮添加中断处理函数
}

