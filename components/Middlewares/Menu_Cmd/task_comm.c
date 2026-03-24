#include "task_comm.h"
#include "esp_log.h"

static const char *TAG = "task_comm";

// 全局变量定义
QueueHandle_t g_task_queue = NULL;
EventGroupHandle_t g_event_group = NULL;
SemaphoreHandle_t g_i2c_mutex = NULL;
SemaphoreHandle_t g_display_mutex = NULL;

/**
 * @brief 初始化任务通信机制
 * @return void
 */
void task_comm_init(void)
{
    // 创建消息队列
    g_task_queue = xQueueCreate(10, sizeof(task_msg_t));
    if (g_task_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create task queue");
        return;
    }
    ESP_LOGI(TAG, "Task queue created successfully");

    // 创建事件组
    g_event_group = xEventGroupCreate();
    if (g_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create event group");
        return;
    }
    ESP_LOGI(TAG, "Event group created successfully");

    // 创建I2C互斥锁
    g_i2c_mutex = xSemaphoreCreateMutex();
    if (g_i2c_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create I2C mutex");
        return;
    }
    ESP_LOGI(TAG, "I2C mutex created successfully");

    // 创建显示互斥锁
    g_display_mutex = xSemaphoreCreateMutex();
    if (g_display_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create display mutex");
        return;
    }
    ESP_LOGI(TAG, "Display mutex created successfully");

    ESP_LOGI(TAG, "Task communication initialized");
}

/**
 * @brief 发送任务消息
 * @param type 消息类型
 * @param param1 参数1
 * @param param2 参数2
 * @param data 附加数据
 * @return void
 */
void send_task_msg(msg_type_t type, uint32_t param1, uint32_t param2, void *data)
{
    task_msg_t msg;
    msg.type = type;
    msg.param1 = param1;
    msg.param2 = param2;
    msg.data = data;

    if (xQueueSend(g_task_queue, &msg, portMAX_DELAY) != pdPASS) {
        ESP_LOGE(TAG, "Failed to send task message");
    }
}

/**
 * @brief 等待事件
 * @param bits 要等待的事件位
 * @param timeout 超时时间
 * @return bool 是否成功等待到事件
 */
bool wait_for_event(EventBits_t bits, TickType_t timeout)
{
    EventBits_t result = xEventGroupWaitBits(g_event_group, bits, pdTRUE, pdFALSE, timeout);
    return (result & bits) == bits;
}

/**
 * @brief 设置事件
 * @param bits 要设置的事件位
 * @return void
 */
void set_event(EventBits_t bits)
{
    xEventGroupSetBits(g_event_group, bits);
}
