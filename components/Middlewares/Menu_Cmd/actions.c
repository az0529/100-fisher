#include "menu.h"
#include "ultrasonic.h"
#include "settings.h"
 
// action1: 模式1操作
void action1(void)
{
    float temp;
    OLED_Clear();
    int time_update_count = 0;
    while (1) {
        // 每10次循环更新一次时间（约每秒一次）
        if (time_update_count % 10 == 0) {
            showtime();
        }
        time_update_count++;
        
        temp = DS18B20_GetTemperature();
        OLED_ShowString(2, 2, "TEMP:", 8);
        OLED_ShowFloat(36, 2, temp, 2, 2, 8);
        OLED_ShowChar(74, 2, 'C', 8);
        
        float distance = ultrasonic_measure_distance();
        OLED_ShowString(2, 4, "DIST:", 8);
        if (distance >= 0) {
            OLED_ShowFloat(40, 4, distance, 2, 1, 8);
            OLED_ShowString(74, 4, "cm", 8);
        } else {
            OLED_ShowString(32, 4, "ERR", 8);
        }
        
        // 检查按键是否按下
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            // 停止采样任务
            ultrasonic_stop_sampling();
            return; // 退出函数，回到菜单选择
        }
        
        // 添加延时，避免CPU占用过高
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
 
// action2: 模式2操作
void action2(void)
{
    OLED_Clear();
    OLED_ShowString(24, 5, "Mode 2 Active", 8); // 居中显示
    // 定义一个简单的图片数据（16x16像素）
    const uint8_t example_image[] = {
        // 第一页（行0-7）
        0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x00, 0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x00,
        // 第二页（行8-15）
        0x00, 0x00, 0x18, 0x24, 0x24, 0x24, 0x18, 0x00, 0x00, 0x00, 0x18, 0x24, 0x24, 0x24, 0x18, 0x00
    };

    // 显示图片（从左上角开始，16x16像素）
    OLED_ShowImage(0, 0, 16, 16, example_image);
    OLED_ShowString(26, 6, "Press to exit", 8); // 居中显示
    
    // 等待按键按下退出
    int time_update_count = 0;
    while (1) {
        // 每10次循环更新一次时间（约每秒一次）
        if (time_update_count % 10 == 0) {
            showtime();
        }
        time_update_count++;
        
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            // 停止采样任务
            ultrasonic_stop_sampling();
            return; // 退出函数，回到菜单选择
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 短延时，避免占用CPU
    }
}
 
/**
 * @brief 模式3操作 - 30秒密度分析
 * @description 执行30秒的超声波采样，分析运动物体的密度和运动变化
 * @return 无
 * @note 该函数会：
 *       1. 启动30秒的超声波采样任务
 *       2. 显示工作提示、当前距离和倒计时
 *       3. 支持按键优先退出
 *       4. 时间结束后显示密度和变化率等级
 *       5. 等待按键按下退出
 */     
void action3(void)
{
    OLED_Clear();
    // 显示静态文本
    OLED_ShowString(0, 1, "30s Density Analysis", 8);
    OLED_ShowString(0, 2, "Working...", 8);
    OLED_ShowString(0, 4, "Distance:", 8);
    OLED_ShowString(0, 6, "Time left: 30 s", 8);
    OLED_ShowString(0, 7, "Press to exit", 8);
    // 显示时间
    showtime();
    
    esp_err_t err = ultrasonic_start_30s_sampling();
    if (err != ESP_OK) {
        OLED_ShowString(0, 4, "Failed to start sampling", 8);
        vTaskDelay(pdMS_TO_TICKS(2000)); // 延时2秒
        button_pressed_flags = 0; // 重置按键标志
        return; // 退出函数
    }
    
    const int total_seconds = 30;
    int countdown = total_seconds; 
    uint32_t start_time = esp_timer_get_time() / 1000; // 开始时间（毫秒）
    
    while (esp_timer_get_time() / 1000 - start_time < total_seconds * 1000) {
        // 计算当前剩余时间
        int current_seconds = total_seconds - (esp_timer_get_time() / 1000 - start_time) / 1000;
        
        // 每秒更新一次倒计时
        if (current_seconds != countdown) {
            countdown = current_seconds;
            // 清除"30"的位置
            OLED_ClearArea(60, 6, 16, 1); // 清除倒计时数字区域
            char time_str[20];
            sprintf(time_str, " %d", countdown); // 格式化倒计时数字
            OLED_ShowString(60, 6, time_str, 8); // 显示剩余时间
        }
        
        // 每秒更新一次时间
        if ((esp_timer_get_time() / 1000 - start_time) % 1000 < 100) {
            // 清除时间区域
            OLED_ClearArea(40, 0, 40, 1);
            // 更新时间
            showtime();
        }
        
        // 从队列中获取距离数据（非阻塞）
        float current_distance = -1;
        if (xQueueReceive(ultrasonic_get_distance_queue(), &current_distance, 0) == pdPASS) {
            // 清除距离显示区域
            OLED_ClearArea(48, 4, 40, 1); // 清除距离显示区域
            if (current_distance >= 0) {
                OLED_ShowFloat(48, 4, current_distance, 2, 1, 8);
                OLED_ShowString(74, 4, "cm", 8);
            } else {
                OLED_ShowString(48, 4, "ERR", 8);
            }
        }
        
        // 检查按键是否按下
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            // 停止采样任务
            ultrasonic_stop_sampling();
            return; // 退出函数，回到菜单选择
        }
        
        // 添加延时，避免CPU占用过高
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // 获取采样结果
    int density; // 密度等级（0-低，1-中，2-高）
    float motion_change; // 运动变化（方差值）
    err = ultrasonic_get_sampling_result(&density, &motion_change);
    
    OLED_Clear();
    OLED_ShowString(0, 0, "Analysis Result", 8);
    
    if (err == ESP_OK) {
        OLED_ShowString(0, 2, "Density:", 8);
        switch (density) {
            case 0:
                OLED_ShowString(45, 2, "Low", 8); // 低密度
                break;
            case 1:
                OLED_ShowString(45, 2, "Medium", 8); // 中密度
                break;
            case 2:
                OLED_ShowString(45, 2, "High", 8); // 高密度
                break;
        }
        
        OLED_ShowString(0, 4, "Motion:", 8);
        if (motion_change < 100) {
            OLED_ShowString(45, 4, "Small", 8); // 小变化
        } else if (motion_change < 500) {
            OLED_ShowString(45, 4, "Medium", 8); // 中变化
        } else {
            OLED_ShowString(45, 4, "Large", 8); // 大变化
        }
    } else {
        // 分析失败，显示错误信息
        OLED_ShowString(0, 2, "Analysis failed", 8);
    }
    
    OLED_ShowString(0, 6, "Press to exit", 8);
    
    // 等待按键按下退出
    while (1) {
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            return; // 退出函数，回到菜单选择
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 短延时，避免占用CPU
    }
}
 


// action4: 模式4操作 - 设置选项
void action4(void)
{
    execute_settings_menu();
}

// action5: 模式5操作
void action5(void)
{
    OLED_Clear();
    OLED_ShowString(24, 2, "Mode 5 Active", 8); // 居中显示
    OLED_ShowString(26, 6, "Press to exit", 8); // 居中显示
    
    // 等待按键按下退出
    int time_update_count = 0;
    while (1) {
        // 每10次循环更新一次时间（约每秒一次）
        if (time_update_count % 10 == 0) {
            showtime();
        }
        time_update_count++;
        
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            return; // 退出函数，回到菜单选择
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 短延时，避免占用CPU
    }
}

// action6: 模式6操作
void action6(void)
{
    OLED_Clear();
    OLED_ShowString(24, 2, "Mode 6 Active", 8); // 居中显示
    OLED_ShowString(26, 6, "Press to exit", 8); // 居中显示
    
    // 等待按键按下退出
    int time_update_count = 0;
    while (1) {
        // 每10次循环更新一次时间（约每秒一次）
        if (time_update_count % 10 == 0) {
            showtime();
        }
        time_update_count++;
        
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            return; // 退出函数，回到菜单选择
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 短延时，避免占用CPU
    }
}

// action7: 模式7操作
void action7(void)
{
    OLED_Clear();
    OLED_ShowString(24, 2, "Mode 7 Active", 8); // 居中显示
    OLED_ShowString(26, 6, "Press to exit", 8); // 居中显示
    
    // 等待按键按下退出
    int time_update_count = 0;
    while (1) {
        // 每10次循环更新一次时间（约每秒一次）
        if (time_update_count % 10 == 0) {
            showtime();
        }
        time_update_count++;
        
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            return; // 退出函数，回到菜单选择
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 短延时，避免占用CPU
    }
}

// action8: 模式8操作
void action8(void)
{
    OLED_Clear();
    OLED_ShowString(24, 2, "Mode 8 Active", 8); // 居中显示
    OLED_ShowString(26, 6, "Press to exit", 8); // 居中显示
    
    // 等待按键按下退出
    int time_update_count = 0;
    while (1) {
        // 每10次循环更新一次时间（约每秒一次）
        if (time_update_count % 10 == 0) {
            showtime();
        }
        time_update_count++;
        
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            return; // 退出函数，回到菜单选择
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 短延时，避免占用CPU
    }
}

// action9: 模式9操作
void action9(void)
{
    OLED_Clear();
    OLED_ShowString(24, 2, "Mode 9 Active", 8); // 居中显示
    OLED_ShowString(26, 6, "Press to exit", 8); // 居中显示
    
    // 等待按键按下退出
    int time_update_count = 0;
    while (1) {
        // 每10次循环更新一次时间（约每秒一次）
        if (time_update_count % 10 == 0) {
            showtime();
        }
        time_update_count++;
        
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            return; // 退出函数，回到菜单选择
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 短延时，避免占用CPU
    }
}

// action10: 模式10操作
void action10(void)
{
    OLED_Clear();
    OLED_ShowString(24, 2, "Mode 10 Active", 8); // 居中显示
    OLED_ShowString(26, 6, "Press to exit", 8); // 居中显示
    
    // 等待按键按下退出
    int time_update_count = 0;
    while (1) {
        // 每10次循环更新一次时间（约每秒一次）
        if (time_update_count % 10 == 0) {
            showtime();
        }
        time_update_count++;
        
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            return; // 退出函数，回到菜单选择
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 短延时，避免占用CPU
    }
}

void showtime(void)
{
    time_t now = get_current_time();
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char time_str[20];
    sprintf(time_str, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    OLED_ShowString(40, 0, time_str, 8);
}

void if_exit(void)
{
    while (1) {
        if (button_pressed_flags != 0) {
            button_pressed_flags = 0; // 重置按键标志
            return; // 退出函数，回到菜单选择
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 短延时，避免占用CPU
    }
}