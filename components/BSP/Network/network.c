#include "network.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

// 事件组
static EventGroupHandle_t s_wifi_event_group;

// 事件位
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

// 日志标签
static const char *TAG = "network";

/**
 * @brief WiFi事件处理函数
 * @param arg 传递给事件处理函数的参数
 * @param event_base 事件基类（如WIFI_EVENT或IP_EVENT）
 * @param event_id 事件ID
 * @param event_data 事件数据
 * @description 处理WiFi相关的事件，包括：
 *              1. WiFi STA模式启动时，尝试连接WiFi
 *              2. WiFi断开连接时，重新尝试连接
 *              3. 获取到IP地址时，记录IP地址并设置连接成功的事件位
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base, 
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "retry to connect to the AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief 初始化WiFi并连接到指定的AP
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 * @description 初始化WiFi网络接口，配置为STA模式，连接到指定的WiFi网络
 * @note 需要在network.h中配置正确的WIFI_SSID和WIFI_PASSWORD
 */
esp_err_t wifi_init(void)
{
    s_wifi_event_group = xEventGroupCreate();

    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, 
                                                      ESP_EVENT_ANY_ID, 
                                                      &wifi_event_handler, 
                                                      NULL, 
                                                      &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, 
                                                      IP_EVENT_STA_GOT_IP, 
                                                      &wifi_event_handler, 
                                                      NULL, 
                                                      &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    // 等待连接成功或失败
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, 
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, 
                                           pdFALSE, 
                                           pdFALSE, 
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", 
                 WIFI_SSID, WIFI_PASSWORD);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", 
                 WIFI_SSID, WIFI_PASSWORD);
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        return ESP_FAIL;
    }
}

/**
 * @brief 同步NTP时间（异步）
 * @return esp_err_t - ESP_OK表示成功，其他值表示失败
 * @description 初始化SNTP服务并开始同步，但不等待同步完成
 * @note 需要在network.h中配置正确的NTP_SERVER
 */
esp_err_t ntp_sync_time(void)
{
    // 初始化SNTP
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);  // 设置为轮询模式
    
    // 设置多个NTP服务器，提高同步成功率
    esp_sntp_setservername(0, "ntp.aliyun.com");   // 阿里云NTP服务器（国内速度快）
    esp_sntp_setservername(1, "ntp.tencent.com");  // 腾讯云NTP服务器（国内速度快）
    esp_sntp_setservername(2, "pool.ntp.org");     // 全球NTP服务器（备用）
    
    // 设置同步间隔为1小时
    esp_sntp_set_sync_interval(3600000);
    
    esp_sntp_init();  // 初始化SNTP服务

    // 设置时区为北京时间（UTC+8）
    setenv("TZ", "CST-8", 1);
    tzset();  // 应用时区设置

    ESP_LOGI(TAG, "NTP synchronization started");
    return ESP_OK;
}

/**
 * @brief 检查NTP同步状态
 * @return bool - true表示已同步，false表示未同步
 * @description 检查SNTP是否已完成时间同步
 */
bool is_ntp_synchronized(void)
{
    return esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED;
}

/**
 * @brief 获取当前时间（带回退机制）
 * @return time_t - 当前时间戳
 * @description 如果NTP已同步，返回NTP时间；否则返回系统时间
 */
time_t get_current_time(void)
{
    time_t now;
    time(&now);
    return now;
}