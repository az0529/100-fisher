#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "esp_err.h"
#include "time.h"

// WiFi配置
#define WIFI_SSID "HONOR 200 Pro"
#define WIFI_PASSWORD "88888888"

// NTP服务器
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 8 * 3600 // 北京时间，UTC+8
#define DAYLIGHT_OFFSET_SEC 0

// 函数声明
esp_err_t wifi_init(void);
esp_err_t ntp_sync_time(void);
time_t get_current_time(void);

#endif