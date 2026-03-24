#ifndef __TASK_COMM_H__
#define __TASK_COMM_H__

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

// 消息类型定义
typedef enum {
    MSG_TYPE_MENU_SELECT,      // 菜单选择消息
    MSG_TYPE_ENCODER_INPUT,    // 编码器输入消息
    MSG_TYPE_DATA_UPDATE,      // 数据更新消息
    MSG_TYPE_MODE_CHANGE,      // 模式切换消息
    MSG_TYPE_SAMPLING_DONE,    // 采样完成消息
    MSG_TYPE_ERROR             // 错误消息
} msg_type_t;

// 消息结构体
typedef struct {
    msg_type_t type;           // 消息类型
    uint32_t param1;           // 参数1
    uint32_t param2;           // 参数2
    void *data;                // 附加数据
} task_msg_t;

// 事件组定义
#define EVENT_ENCODER_PRESSED    (1 << 0)  // 编码器按键按下
#define EVENT_DATA_READY         (1 << 1)  // 数据准备就绪
#define EVENT_SAMPLING_DONE      (1 << 2)  // 采样完成
#define EVENT_MODE_CHANGED       (1 << 3)  // 模式改变

// 外部声明
extern QueueHandle_t g_task_queue;
extern EventGroupHandle_t g_event_group;
extern SemaphoreHandle_t g_i2c_mutex;
extern SemaphoreHandle_t g_display_mutex;

// 函数声明
void task_comm_init(void);
void send_task_msg(msg_type_t type, uint32_t param1, uint32_t param2, void *data);
bool wait_for_event(EventBits_t bits, TickType_t timeout);
void set_event(EventBits_t bits);

#endif