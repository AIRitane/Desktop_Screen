#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "beep.h"
#include "uc8151d.h"
#include "esp_spiffs.h"
#include "wifi.h"
#include "nvs_flash.h"
#include "page.h"
#include "api.h"
#include "sntp.h"

#define WIFI_SSID "li"
#define WIFI_PASS "1234567"

const static char *TAG = "MAIN APP";

void on_wifi_callback(wifi_event_type_t event)
{
    static uint8_t wifi_flag1 = 0;
    static uint8_t wifi_flag2 = 0;
    if (event == EVENT_USER_ONNVS)
    {
        is_connect = 0;
        send_page_event(PAGE_1);
    }
    if (event == EVENT_USER_HASNVS || wifi_flag1)
    {
        wifi_flag1 = 1;
        if (event == EVENT_DISCONNECT)
        {
            is_connect = 0;
            send_page_event(PAGE_2);
        }
    }
    if ((event == EVENT_USER_LOGIN) || wifi_flag2)
    {
        wifi_flag1 = 0;
        wifi_flag2 = 1;
        if (event == EVENT_DISCONNECT)
        {
            is_connect = 0;
            send_page_event(PAGE_3);
        }

        else if (event == EVENT_GOT_IP)
        {
            // obtain_time();
            is_connect = 1;
            send_page_event(PAGE_4);
        }
    }
    if (event == EVENT_GOT_IP || !wifi_flag2)
    {
        is_connect = 1;
        // obtain_time();
        send_page_event(PAGE_4);
    }
}

static wifi_t wifi = {
    .ssid = WIFI_SSID,
    .password = WIFI_PASS,
    .mode = WIFI_MODE_APSTA,
    .netif = NULL,
    .callback = on_wifi_callback};

void app_main(void)
{
    page_init();
    wifi_start(&wifi);

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// AP站的api接口函数
// 基本UI搭建完成