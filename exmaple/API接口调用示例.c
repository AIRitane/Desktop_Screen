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
#include "api.h"
#include "tp.h"
#include "driver/timer.h"
#include "esp_timer.h"
#include "ssqt.h"

const static char *TAG = "MAIN APP";
sntp_time_t * _time_;
yiketianqi_t* weather;
char *word;
char *one;

void app_main(void)
{
    beep_init();

    api_init();
    while (1)
    {
        if (get_if_connect())
        {
            send_api_event(API_TIME);
            _time_ = get_api_time();
            printf("%s\n",_time_->year);
            send_api_event(API_ONE);
            one = get_api_one();
            printf("%s\n",one);
            set_api_local("隆昌");
            send_api_event(API_WEATHER);
            weather = get_api_weather();
            printf("%s\n",weather->win_speed);
            set_api_word("the");
            send_api_event(API_WORD);
            word = get_api_word();
            printf("%s\n",word);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

        