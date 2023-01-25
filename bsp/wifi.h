/*
 * wifi.h
 *
 *  Created on: 2021年7月5日
 *      Author: jenson
 */


#ifndef __WIFI_H__
#define __WIFI_H__

#include "esp_wifi.h"

#define AP_WIFI_SSID 				"Lulincy" //创建出来的AP模式WIFI(热点)
#define AP_WIFI_PASS 				"123456780"     //AP模式密码
#define AP_WIFI_CHANNEL             13
#define AP_MAX_STA_CONN             3

#define CONNECT_HTTP_IP     "192.168.2.1"
#define CONNECT_HTTP_GW     "192.168.2.1"
#define CONNECT_HTTP_MASK   "255.255.255.0"
/**
 * @brief WiFi事件类型
 * */
typedef enum{
    EVENT_GOT_IP, /*获取IP*/
    EVENT_CONNECTED, /*WiFi连接*/
    EVENT_DISCONNECT, /*WiFi断开连接*/
    EVENT_USER_LOGIN,
    EVENT_USER_ONNVS,
    EVENT_USER_HASNVS
}wifi_event_type_t;

typedef enum{
    NVS_WIFI_INFO_ERROR =0,
    NVS_WIFI_INFO_NULL,
    NVS_WIFI_INFO_SAVE,
}NVS_WIFI_INFO_E;

/**
 * @brief WiFi事件回调函数
 * @param event WiFi事件
 * */
typedef void (*on_wifi_event_callback)(wifi_event_type_t event);

/**
 * @brief WiFi设备定义
 * */
typedef struct {
    char* ssid; /*所要连接的SSID*/
    char* password; /*密码*/
    wifi_mode_t mode; /*WiFi工作模式*/
    esp_netif_t* netif; /*WiFi网卡对象*/
    esp_netif_t* ap;
    on_wifi_event_callback callback; /*事件回调对象*/
}wifi_t;

/**
 * @brief 启动WiFi连接
 * @param wifi WiFi设备对象
 * */
void wifi_start(wifi_t* wifi);

/**
 * @brief 关闭WiFi连接
 * @param wifi WiFi设备对象
 * */
void wifi_stop(wifi_t* wifi);

#endif
