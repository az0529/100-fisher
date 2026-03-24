#include "menu.h"
#include "settings.h"
#include "task_comm.h"
#include "esp_log.h"

static const char *TAG = "menu";

// 外部全局数据
extern system_data_t g_system_data;



// 菜单列表定义
char *menu_list[] = 
{
    "01. mode 01",
    "02. mode 02",
    "03. mode 03",
    "04. mode 04",
    "05. mode 05",
    "06. mode 06",
    "07. mode 07",
    "08. mode 08",
    "09. mode 09",
    "10. mode 10",
};

// 动作函数指针数组
action_func_t action_functions[] = {
    action1,
    action2,
    action3,
    action4,
    action5,
    action6,
    action7,
    action8,
    action9,
    action10
};

/**
 * @brief 菜单初始化函数
 * @details 初始化菜单系统，包括编码器GPIO、OLED显示，并显示初始菜单项
 * @return 无
 */
void menu_init()
{
    // 初始化设置
    settings_init();
    
    OLED_ShowString(15, 0, "Menu Initialized",8);
    led_on();  
    vTaskDelay(pdMS_TO_TICKS(500));
    OLED_Clear();
    current_mode=0;  // 设置初始模式为0
    g_system_data.current_mode = current_mode;
    
    ESP_LOGI(TAG, "Menu system initialized");
}




/**
 * @brief 菜单更新函数
 * @details 处理菜单选择、确认和超时逻辑
 * @return 无
 */
void menu_update()
{
    static bool is_in_menu = true;          // 标记是否在菜单选择状态
    static uint8_t last_mode = 0;           // 保存进入模式前的模式
    const TickType_t menu_delay = 10;       // 菜单循环延时
    
    while (1) {
        // 检查消息队列
        task_msg_t msg;
        if (xQueueReceive(g_task_queue, &msg, 0) == pdPASS) {
            switch (msg.type) {
                case MSG_TYPE_ENCODER_INPUT:
                    if (is_in_menu) {
                        // 处理菜单选择
                        if (xSemaphoreTake(g_display_mutex, portMAX_DELAY) == pdTRUE) {
                            OLED_ShowString(10, 1, "Entering Mode...", 8);
                            xSemaphoreGive(g_display_mutex);
                        }
                        last_mode = current_mode;
                        is_in_menu = false;
                        
                        // 更新全局模式
                        g_system_data.current_mode = current_mode;
                        
                        // 设置模式改变事件
                        set_event(EVENT_MODE_CHANGED);
                    } else {
                        // 退出当前模式，返回菜单
                        OLED_Clear();
                        ultrasonic_stop_sampling();
                        is_in_menu = true;
                        
                        // 更新全局模式
                        g_system_data.current_mode = 0;
                    }
                    break;
                case MSG_TYPE_MENU_SELECT:
                    current_mode = msg.param1;
                    g_system_data.current_mode = current_mode;
                    break;
                default:
                    break;
            }
        }
        
        if (is_in_menu) {
            // 显示菜单
            if (xSemaphoreTake(g_display_mutex, portMAX_DELAY) == pdTRUE) {
                showtime();
                OLED_ShowString(30, 1, menu_list[current_mode], 8);  // 索引从0开始
                xSemaphoreGive(g_display_mutex);
            }
        } else {
            // 执行模式操作
            mode_action(current_mode);
            
            // 模式操作完成后，清空显示并返回菜单
            OLED_Clear();
            // 确保所有资源都已释放
            ultrasonic_stop_sampling();
            is_in_menu = true;  // 重新进入菜单选择状态
            
            // 更新全局模式
            g_system_data.current_mode = 0;
        }
        
        // 短暂延时，避免循环过快
        vTaskDelay(menu_delay);
    }
}

void mode_action(uint8_t current_mode)
{
    if (current_mode < sizeof(action_functions) / sizeof(action_functions[0])) {
        // 使用函数指针数组调用动作函数
        action_functions[current_mode]();
    } else {
        // 默认操作
        if (xSemaphoreTake(g_display_mutex, portMAX_DELAY) == pdTRUE) {
            OLED_Clear();
            OLED_ShowString(15, 2, "Unknown Mode", 8);
            xSemaphoreGive(g_display_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}