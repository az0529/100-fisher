#include "settings.h"

// 全局变量，用于存储设置状态
uint8_t power_mode = 0;      // 0: Normal, 1: Light Sleep, 2: Deep Sleep
uint8_t night_fishing_enabled = 0;  // 0: Off, 1: On
uint8_t strong_wind_enabled = 0;    // 0: Off, 1: On


// 显示设置菜单选项
/**
 * @brief 显示设置菜单选项
 * @description 在OLED屏幕上显示设置主菜单，包括功耗模式、夜钓模式和大风模式选项
 * @param current_option 当前选中的菜单选项索引（0: Power Mode, 1: Night Fishing, 2: Strong Wind）
 * @param last_option 上一次选中的菜单选项索引，用于局部擦除
 * @return 无
 * @note 显示当前选中的选项，并用箭头指示
 */
void show_settings_menu(uint8_t current_option, uint8_t last_option)
{
    // 首次显示时整页擦除
    if (last_option == 0xff) {
        OLED_Clear();
        OLED_ShowString(0, 0, "Settings Menu", 8);
        
        // 显示菜单选项
        OLED_ShowString(0, 2, "1. Power Mode", 8);
        OLED_ShowString(0, 4, "2. Night Fishing", 8);
        OLED_ShowString(0, 6, "3. Strong Wind", 8);
    } else {
        // 局部擦除上一次的标志
        switch (last_option) {
            case 0:
                OLED_ClearArea(84, 2, 8, 8); // 擦除"1. Power Mode"后面的位置
                break;
            case 1:
                OLED_ClearArea(102, 4, 8, 8); // 擦除"2. Night Fishing"后面的位置
                break;
            case 2:
                OLED_ClearArea(90, 6, 8, 8); // 擦除"3. Strong Wind"后面的位置
                break;
        }
    }
    
    // 显示当前选中的选项，将>放在每个选项后面，与文字保持一定距离
    switch (current_option) {
        case 0:
            OLED_ShowChar(84, 2, '>', 8); // "1. Power Mode"后面的位置
            break;
        case 1:
            OLED_ShowChar(102, 4, '>', 8); // "2. Night Fishing"后面的位置
            break;
        case 2:
            OLED_ShowChar(90, 6, '>', 8); // "3. Strong Wind"后面的位置
            break;
    }
}

// 显示功耗模式菜单
/**
 * @brief 显示功耗模式菜单
 * @description 在OLED屏幕上显示功耗模式子菜单，包括正常模式、轻度睡眠和深度睡眠选项
 * @param current_option 当前选中的功耗模式索引（0: Normal, 1: Light Sleep, 2: Deep Sleep）
 * @param last_option 上一次选中的功耗模式索引，用于局部擦除
 * @return 无
 * @note 显示当前选中的功耗模式，并用箭头指示
 */
void show_power_mode_menu(uint8_t current_option, uint8_t last_option)
{
    // 首次显示时整页擦除
    if (last_option == 0xff) {
        OLED_Clear();
        OLED_ShowString(0, 0, "Power Mode", 8);
        
        // 显示功耗模式选项
        OLED_ShowString(0, 2, "1. Normal", 8);
        OLED_ShowString(0, 4, "2. Light Sleep", 8);
        OLED_ShowString(0, 6, "3. Deep Sleep", 8);
    } else {
        // 局部擦除上一次的标志
        switch (last_option) {
            case 0:
                OLED_ClearArea(60, 2, 8, 8); // 擦除"1. Normal"后面的位置
                break;
            case 1:
                OLED_ClearArea(90, 4, 8, 8); // 擦除"2. Light Sleep"后面的位置
                break;
            case 2:
                OLED_ClearArea(84, 6, 8, 8); // 擦除"3. Deep Sleep"后面的位置
                break;
        }
    }
    
    // 显示当前选中的选项，将>放在每个选项后面，与文字保持一定距离
    switch (current_option) {
        case 0:
            OLED_ShowChar(60, 2, '>', 8); // "1. Normal"后面的位置
            break;
        case 1:
            OLED_ShowChar(90, 4, '>', 8); // "2. Light Sleep"后面的位置
            break;
        case 2:
            OLED_ShowChar(84, 6, '>', 8); // "3. Deep Sleep"后面的位置
            break;
    }
}

// 显示夜钓模式菜单
/**
 * @brief 显示夜钓模式菜单
 * @description 在OLED屏幕上显示夜钓模式子菜单，包括关闭和开启选项
 * @param current_option 当前选中的夜钓模式索引（0: Off, 1: On）
 * @param is_enabled 当前夜钓模式的开启状态（0: 关闭, 1: 开启）
 * @param last_option 上一次选中的夜钓模式索引，用于局部擦除
 * @return 无
 * @note 显示当前选中的夜钓模式，并用箭头指示，同时显示当前的开关状态
 */
void show_night_fishing_menu(uint8_t current_option, uint8_t is_enabled, uint8_t last_option)
{
    // 首次显示时整页擦除
    if (last_option == 0xff) {
        OLED_Clear();
        OLED_ShowString(0, 0, "Night Fishing", 8);
        
        // 显示夜钓模式选项
        OLED_ShowString(0, 2, "1. Off", 8);
        OLED_ShowString(0, 4, "2. On", 8);
        
        // 显示开关状态
        if (is_enabled) {
            OLED_ShowCheck(90, 4);
        } else {
            OLED_ShowBox(90, 2);
        }
    } else {
        // 局部擦除上一次的标志
        switch (last_option) {
            case 0:
                OLED_ClearArea(40, 2, 8, 8); // 擦除"1. Off"后面的位置
                break;
            case 1:
                OLED_ClearArea(40, 4, 8, 8); // 擦除"2. On"后面的位置
                break;
        }
    }
    
    // 显示当前选中的选项，将>放在每个选项后面，与文字保持一定距离
    switch (current_option) {
        case 0:
            OLED_ShowChar(40, 2, '>', 8); // "1. Off"后面的位置
            break;
        case 1:
            OLED_ShowChar(40, 4, '>', 8); // "2. On"后面的位置
            break;
    }
}

// 显示大风模式菜单
/**
 * @brief 显示大风模式菜单
 * @description 在OLED屏幕上显示大风模式子菜单，包括关闭和开启选项
 * @param current_option 当前选中的大风模式索引（0: Off, 1: On）
 * @param is_enabled 当前大风模式的开启状态（0: 关闭, 1: 开启）
 * @param last_option 上一次选中的大风模式索引，用于局部擦除
 * @return 无
 * @note 显示当前选中的大风模式，并用箭头指示，同时显示当前的开关状态
 */
void show_strong_wind_menu(uint8_t current_option, uint8_t is_enabled, uint8_t last_option)
{
    // 首次显示时整页擦除
    if (last_option == 0xff) {
        OLED_Clear();
        OLED_ShowString(0, 0, "Strong Wind", 8);
        
        // 显示大风模式选项
        OLED_ShowString(0, 2, "1. Off", 8);
        OLED_ShowString(0, 4, "2. On", 8);
        
        // 显示开关状态
        if (is_enabled) {
            OLED_ShowCheck(90, 4);
        } else {
            OLED_ShowBox(90, 2);
        }
    } else {
        // 局部擦除上一次的标志
        switch (last_option) {
            case 0:
                OLED_ClearArea(40, 2, 8, 8); // 擦除"1. Off"后面的位置
                break;
            case 1:
                OLED_ClearArea(40, 4, 8, 8); // 擦除"2. On"后面的位置
                break;
        }
    }
    
    // 显示当前选中的选项，将>放在每个选项后面，与文字保持一定距离
    switch (current_option) {
        case 0:
            OLED_ShowChar(40, 2, '>', 8); // "1. Off"后面的位置
            break;
        case 1:
            OLED_ShowChar(40, 4, '>', 8); // "2. On"后面的位置
            break;
    }
}

// 初始化设置
void settings_init(void)
{
    // 初始化设置值
    power_mode = 0;      // 默认 Normal
    night_fishing_enabled = 0;  // 默认 Off
    strong_wind_enabled = 0;    // 默认 Off
}

// 执行设置菜单
void execute_settings_menu(void)
{
    uint8_t settings_option = 0;
    uint8_t last_settings_option = 0xff; // 初始值为0xff表示首次显示
    uint8_t last_encoder_value = current_mode;
    show_settings_menu(settings_option, last_settings_option);
    last_settings_option = settings_option;
    
    while (1) {
        // 检查编码器旋转
        if (current_mode != last_encoder_value) {
            last_encoder_value = current_mode;
            // 计算新的选项
            settings_option = (settings_option + 1) % 3;
            show_settings_menu(settings_option, last_settings_option);
            last_settings_option = settings_option;
        }
        
        // 检查按键是否按下
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            
            // 进入下一级菜单
            switch (settings_option) {
                case 0: // 功耗模式
                    {
                        uint8_t power_option = power_mode;
                        uint8_t last_power_option = 0xff; // 初始值为0xff表示首次显示
                        uint8_t last_power_encoder = current_mode;
                        show_power_mode_menu(power_option, last_power_option);
                        last_power_option = power_option;
                        
                        while (1) {
                            // 检查编码器旋转
                            if (current_mode != last_power_encoder) {
                                last_power_encoder = current_mode;
                                // 计算新的选项
                                power_option = (power_option + 1) % 3;
                                show_power_mode_menu(power_option, last_power_option);
                                last_power_option = power_option;
                            }
                            
                            if (button_pressed_flags != 0) {
                                button_pressed_flags = 0; // 重置按键标志
                                
                                // 保存功耗模式
                                power_mode = power_option;
                                
                                OLED_Clear();
                                OLED_ShowString(0, 2, "Power Mode Saved", 8);
                                vTaskDelay(pdMS_TO_TICKS(1000));
                                return; // 自动返回主菜单
                            }
                            vTaskDelay(pdMS_TO_TICKS(100));
                        }
                    }
                case 1: // 夜钓开启与否
                    {
                        uint8_t night_option = night_fishing_enabled;
                        uint8_t last_night_option = 0xff; // 初始值为0xff表示首次显示
                        uint8_t last_night_encoder = current_mode;
                        show_night_fishing_menu(night_option, night_fishing_enabled, last_night_option);
                        last_night_option = night_option;
                        
                        while (1) {
                            // 检查编码器旋转
                            if (current_mode != last_night_encoder) {
                                last_night_encoder = current_mode;
                                // 计算新的选项
                                night_option = (night_option + 1) % 2;
                                show_night_fishing_menu(night_option, night_fishing_enabled, last_night_option);
                                last_night_option = night_option;
                            }
                            
                            if (button_pressed_flags != 0) {
                                button_pressed_flags = 0; // 重置按键标志
                                
                                // 保存夜钓模式
                                night_fishing_enabled = night_option;
                                
                                OLED_Clear();
                                OLED_ShowString(0, 2, "Night Fishing Saved", 8);
                                vTaskDelay(pdMS_TO_TICKS(1000));
                                return; // 自动返回主菜单
                            }
                            vTaskDelay(pdMS_TO_TICKS(100));
                        }
                    }
                case 2: // 大风模式
                    {
                        uint8_t wind_option = strong_wind_enabled;
                        uint8_t last_wind_option = 0xff; // 初始值为0xff表示首次显示
                        uint8_t last_wind_encoder = current_mode;
                        show_strong_wind_menu(wind_option, strong_wind_enabled, last_wind_option);
                        last_wind_option = wind_option;
                        
                        while (1) {
                            // 检查编码器旋转
                            if (current_mode != last_wind_encoder) {
                                last_wind_encoder = current_mode;
                                // 计算新的选项
                                wind_option = (wind_option + 1) % 2;
                                show_strong_wind_menu(wind_option, strong_wind_enabled, last_wind_option);
                                last_wind_option = wind_option;
                            }
                            
                            if (button_pressed_flags != 0) {
                                button_pressed_flags = 0; // 重置按键标志
                                
                                // 保存大风模式
                                strong_wind_enabled = wind_option;
                                
                                OLED_Clear();
                                OLED_ShowString(0, 2, "Strong Wind Saved", 8);
                                vTaskDelay(pdMS_TO_TICKS(1000));
                                return; // 自动返回主菜单
                            }
                            vTaskDelay(pdMS_TO_TICKS(100));
                        }
                    }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}