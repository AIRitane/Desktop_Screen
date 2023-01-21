#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string.h>
#include "uc8151d.h"

const static char *TAG = "MAIN APP";

TaskHandle_t ucTask_Handle = NULL;
#define UC_TASK_HEAP 10000

static void uc8151d_task(void *arg);

void app_main(void)
{
    int buffer_sz = 0;
    xTaskCreatePinnedToCore(uc8151d_task, "uc8151d_task", UC_TASK_HEAP, NULL, 10, ucTask_Handle, 1);

    while (1)
    {
        buffer_sz = UC_TASK_HEAP - uxTaskGetStackHighWaterMark(ucTask_Handle);
        ESP_LOGI(TAG, "buffer_sz = %d", buffer_sz);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void uc8151d_task(void *arg)
{
    char image[5000] = {0};
    char li[1000] = "一ge淑h哈哈凤凰\n大家是否@33斤4；【sdk】few仍无法";
    // ESP_LOGW(TAG,"%s",li);
    screen_init();
    screen_show_chinese(image, 0, 0, li, 1);
    screen_full_display(image);
    while (1)
    {
        // printf("ok\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
