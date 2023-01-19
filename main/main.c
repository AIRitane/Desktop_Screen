#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"

#include "wifi.h"
#include "beep.h"
// #include "https_request.h"
#include "http_request.h"

const static char *TAG = "MAIN APP";

#define WIFI_SSID "li"
#define WIFI_PASS "123456789"

// https_request_t one;
// char host[] = "https://www.ooopn.com";
// uint16_t port = 443;
// char web_path[] = "https://www.ooopn.com/tool/api/yan/api.php?type=json";

http_request_t yiketianqi;
char host[] = "www.yiketianqi.com";
uint16_t port = 80;
char web_path[] = "/free/day?appid=59273188&appsecret=WKGR79FN&unescape=1&city=隆昌";

// void data_handler(struct esp_tls *tls, const char *data, size_t len)
// {

//     printf("%s\n", data);
// }

void data_handler(const char *data, size_t len)
{
    printf("%s\n", data);
}


void on_wifi_callback(wifi_event_type_t event)
{
    if (event == EVENT_GOT_IP)
    {
        yiketianqi.host = host;
        yiketianqi.port = port;
        yiketianqi.web_path = web_path;
        yiketianqi.data_handler = data_handler;


        vTaskDelay(1000 / portTICK_PERIOD_MS);
        start_http_request(&yiketianqi);
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


    while (1)
    {
        // send_beep_event(BEEP_SHORT_100MS);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
