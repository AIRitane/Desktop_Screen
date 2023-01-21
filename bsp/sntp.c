#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "sntp.h"

static const char *TAG = "SNTP";

static void initialize_sntp(void);

void obtain_time(void)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year > (2000 - 1900))
        return;

    initialize_sntp();
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        // ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    if (retry > retry_count)
    {
        ESP_LOGE(TAG, "SNTP初始化失败");
    }

    sntp_stop();
}

static void initialize_sntp(void)
{
    // ESP_LOGI(TAG, "Initializing SNTP");
    // make menuconfig -> Component config -> LWIP -> SNTP -> Maximum bumber of NTP servers 修改为 3
    // make menuconfig -> Component config -> LWIP -> SNTP -> Request interval to update time (ms).
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "ntp1.aliyun.com");
    sntp_setservername(1, "210.72.145.44"); // 国家授时中心服务器 IP 地址
    sntp_setservername(2, "1.cn.pool.ntp.org");
    sntp_init();
}

void sntp_time_fresh(sntp_time_t *sntp_time)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    setenv("TZ", "CST-8", 1);
    tzset();
    localtime_r(&now, &timeinfo);

    strftime(sntp_time->year, sizeof(sntp_time->year), "%Y", &timeinfo);
    strftime(sntp_time->mon, sizeof(sntp_time->mon), "%m", &timeinfo);
    strftime(sntp_time->week, sizeof(sntp_time->week), "%w", &timeinfo);
    strftime(sntp_time->hour, sizeof(sntp_time->hour), "%H", &timeinfo);
    strftime(sntp_time->min, sizeof(sntp_time->min), "%M", &timeinfo);
    strftime(sntp_time->sec, sizeof(sntp_time->sec), "%S", &timeinfo);

    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", sntp_time->year);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", sntp_time->mon);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", sntp_time->week);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", sntp_time->hour);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", sntp_time->min);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", sntp_time->sec);
}