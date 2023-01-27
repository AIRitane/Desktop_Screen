#include "api.h"
#include "https_request.h"
#include "http_request.h"
#include "wifi.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "js.h"
#include "sntp.h"
#include <string.h>

#define TAG "API"
#define API_TASK_HEAP 20000
#define WIFI_SSID "li"
#define WIFI_PASS "123456789"

static xQueueHandle api_queue = NULL;
TaskHandle_t apiTask_Handle = NULL;
uint8_t is_connect = 0;

/*******************************接口API封装******************************************/
/*******************************参数设置函数**************************/
char weather_web_path_base[200] = "/free/day?appid=59273188&appsecret=WKGR79FN&unescape=1&city=";
char word_web_path_base[200] = "/suggest?num=1&doctype=json&q=";
char weather_web_path[200];
char word_web_path[200];
void set_api_local(char *local)
{
    memset(weather_web_path,0,sizeof(weather_web_path));
    memcpy(weather_web_path, weather_web_path_base, strlen(weather_web_path_base));
    strcat(weather_web_path, local);
}

void set_api_word(char *word)
{
    memset(word_web_path,0,sizeof(word_web_path));
    memcpy(word_web_path, word_web_path_base, strlen(word_web_path_base));
    strcat(word_web_path, word);
}

/*******************************回调函数*****************************/
char one_buf[200] = {0};
yiketianqi_t yiketianqi_buf = {0};
char word_buf[200] = {0};
sntp_time_t sntp_time = {0};

void one_data_handler(struct esp_tls *tls, const char *data, size_t len)
{
    one_js(data, one_buf);
}

void weather_data_handler(const char *data, size_t len)
{
    yiketianqi_js(data, &yiketianqi_buf);
}

void word_data_handler(const char *data, size_t len)
{
    word_js(data, &word_buf);
}

/*******************************访问函数*****************************/
static void api_one()
{
    https_request_t one;
    one.host = "https://www.ooopn.com";
    one.port = 443;
    one.web_path = "https://www.ooopn.com/tool/api/yan/api.php?type=json";
    one.data_handler = one_data_handler;

    start_https_request(&one);
}

static void api_weather()
{
    http_request_t weather;
    weather.host = "www.yiketianqi.com";
    weather.port = 80;
    weather.web_path = weather_web_path;
    weather.data_handler = weather_data_handler;

    start_http_request(&weather);
}

static void api_wold()
{
    http_request_t word;
    word.host = "dict.youdao.com";
    word.port = 80;
    word.web_path = word_web_path;
    word.data_handler = word_data_handler;

    start_http_request(&word);
}

static void api_time()
{
    sntp_time_fresh(&sntp_time);
}

/*******************************外部接口函数*****************************/
char *get_api_one()
{
    return one_buf;
}
char *get_api_word()
{
    return word_buf;
}
yiketianqi_t *get_api_weather()
{
    return &yiketianqi_buf;
}
sntp_time_t *get_api_time()
{
    return &sntp_time;
}
uint8_t get_if_connect()
{
    return is_connect;
}

/***********************************任务函数******************************************/
static void api_task(void *arg)
{
    uint32_t evt;
    int buffer_sz = 0;
    while (1)
    {
        buffer_sz = API_TASK_HEAP - uxTaskGetStackHighWaterMark(apiTask_Handle);
        // ESP_LOGW(TAG, "api堆栈剩余大小 = %d\n", buffer_sz);
        if (buffer_sz <= 400)
        {
            ESP_LOGW(TAG, "api堆栈剩余大小 = %d\n", buffer_sz);
        }
        if (xQueueReceive(api_queue, &evt, portMAX_DELAY))
        {
            if (is_connect == 0)
            {
                continue;
            }

            switch (evt)
            {
            case API_TIME:
                api_time();
                break;
            case API_ONE:
                api_one();
                break;
            case API_WEATHER:
                api_weather();
                break;
            case API_WORD:
                api_wold();
                break;
            default:
                break;
            }
        }
    }
}

/*******************************API初始化函数******************************************/
void api_init()
{
    api_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreatePinnedToCore(api_task, "api_task", API_TASK_HEAP, NULL, 5, apiTask_Handle, 0);
}

/********************************API事件函数******************************************/
void send_api_event(API_TYPE_E type)
{
    uint32_t evt = type;
    xQueueSend(api_queue, &evt, 0);
}

void send_api_event_from_isr(API_TYPE_E type)
{
    uint32_t evt = type;
    xQueueSendFromISR(api_queue, &evt, 0);
}
