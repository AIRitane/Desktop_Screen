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

#define WIFI_SSID "li"
#define WIFI_PASS "1234567"

const static char *TAG = "MAIN APP";

void on_wifi_callback(wifi_event_type_t event)
{
}

static wifi_t wifi = {
    .ssid = WIFI_SSID,
    .password = WIFI_PASS,
    .mode = WIFI_MODE_APSTA,
    .netif = NULL,
    .callback = on_wifi_callback};


char *ssid = NULL, *psw = NULL;
void app_main(void)
{

    beep_init();

    wifi_start(&wifi);
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// AP站的api接口函数
// 基本UI搭建完成