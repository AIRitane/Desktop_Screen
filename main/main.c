#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string.h>
#include "https_request.h"
#include "http_request.h"
#include "beep.h"
#include "wifi.h"
#include "js.h"
#include "sntp.h"

const static char *TAG = "MAIN APP";

#define WIFI_SSID "li"
#define WIFI_PASS "123456789"

// https_request_t one;
// char host[] = "https://www.ooopn.com";
// uint16_t port = 443;
// char web_path[] = "https://www.ooopn.com/tool/api/yan/api.php?type=json";

// http://quan.suning.com/getSysTime.do
https_request_t one;
char host[] = "https://api.uukit.com";
uint16_t port = 443;
char web_path[] = "https://api.uukit.com/time";
// yiketianqi_t _yiketianqi;

// void data_handler(const char *data, size_t len)
// {
//     // yiketianqi_js(data,&_yiketianqi);
//     // printf("%s\n", _yiketianqi.air);
//     // printf("%s\n", _yiketianqi.city);
//     // printf("%s\n", _yiketianqi.humidity);
//     // printf("%s\n", _yiketianqi.pressure);
//     // printf("%s\n", _yiketianqi.tem);
//     // printf("%s\n", _yiketianqi.tem_day);
//     // printf("%s\n", _yiketianqi.tem_night);
//     // printf("%s\n", _yiketianqi.wea);
//     // printf("%s\n", _yiketianqi.wea_img);
//     // printf("%s\n", _yiketianqi.win);
//     // printf("%s\n", _yiketianqi.win_meter);
//     // printf("%s\n", _yiketianqi.win_speed);
//     printf("%s\n",data);
// }

void data_handler(struct esp_tls *tls, const char *data, size_t len)
{
    // char buf[200];
    // one_js(data,buf);
    printf("%s\n", data);
}

void on_wifi_callback(wifi_event_type_t event)
{
    if (event == EVENT_GOT_IP)
    {
        // one.host = host;
        // one.port = port;
        // one.web_path = web_path;
        // one.data_handler = data_handler;

        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        // start_https_request(&one);
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        // Is time set? If not, tm_year will be (1970 - 1900).
        if (timeinfo.tm_year < (2000 - 1900))
            obtain_time();
    }
}

void app_main(void)
{
    static wifi_t wifi = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
        .mode = WIFI_MODE_STA,
        .netif = NULL,
        .callback = on_wifi_callback};
    beep_init();
    wifi_start(&wifi);

    sntp_time_t sntp_time;
    while (1)
    {
        sntp_time_fresh(&sntp_time);

        send_beep_event(BEEP_SHORT_100MS);
        vTaskDelay(20000 / portTICK_PERIOD_MS);
    }
}
