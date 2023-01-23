#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "beep.h"
#include "uc8151d.h"
#include "esp_spiffs.h"

const static char *TAG = "MAIN APP";
char buf[5000] = {0};
char pic[800] = {0};
char gImage_start[5000];
char gImage_1[472];

void app_main(void)
{
    beep_init();
    screen_init();
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};

    // 挂载注册
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "磁盘挂载失败");
        return NULL;
    }
    // 读取
    FILE *f = fopen("/spiffs/1.bin", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return NULL;
    }
    fread(gImage_1, 1, 20000, f);


    screen_pic_overlay(buf, gImage_1, 20, 20, 64, 59);

    screen_full_display(buf);

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
