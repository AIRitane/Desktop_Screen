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

const static char *TAG = "MAIN APP";
// sntp_time_t * _time_;
// yiketianqi_t* weather;
// char *word;
// char *one;
TouchPoint_T *one;

void app_main(void)
{
    beep_init();
    screen_tp_init();
    while (1)
    {
        one = get_touch_point();
        printf("%d %d %d %d %d %d\n",one->even,one->geste,one->last_x,one->last_y,one->x[0],one->y[0]);
        // send_beep_event(BEEP_SHORT_100MS);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// esp_timer_handle_t esp_timer_handle;

// void tp_timer_callback(void *arg)
// {
//    send_beep_event(BEEP_SHORT_100MS);
// }

// static void tp_timer_init()
// {
//     esp_timer_create_args_t fw_timer = {
// 			.callback = &tp_timer_callback, //定时器回调函数
// 			.arg = NULL,
// 			.name = "beep_timer", //定时器名称
// 	};
 
// 	esp_err_t err = esp_timer_create(&fw_timer,&esp_timer_handle);
// 	err = esp_timer_start_periodic(esp_timer_handle,1000*1000*60); //us级定时，1000*1000=1s
// 	if(err!=ESP_OK){
// 		ESP_LOGE(TAG,"TP定时器初始化错误");
// 	}
// }

 // api_init();
        // if (get_if_connect())
        // {
        //     send_api_event(API_TIME);
        //     _time_ = get_api_time();
        //     printf("%s\n",_time_->year);
        //     send_api_event(API_ONE);
        //     one = get_api_one();
        //     printf("%s\n",one);
        //     set_api_local("隆昌");
        //     send_api_event(API_WEATHER);
        //     weather = get_api_weather();
        //     printf("%s\n",weather->win_speed);
        //     set_api_word("the");
        //     send_api_event(API_WORD);
        //     word = get_api_word();
        //     printf("%s\n",word);
        // }