#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// #ifdef LV_LVGL_H_INCLUDE_SIMPLE
// #include "lvgl.h"
// #else
// #include "lvgl/lvgl.h"
// #endif

#include "uc8151d.h"

const static char *TAG = "MAIN APP";

TaskHandle_t ucTask_Handle = NULL;
#define UC_TASK_HEAP 10000

uint8_t image[5000] = {0xff};
static void uc8151d_task(void *arg);

void app_main(void)
{
    int buffer_sz = 0;
    xTaskCreatePinnedToCore(uc8151d_task, "uc8151d_task", UC_TASK_HEAP, NULL, 10, ucTask_Handle,1);
    while (1)
    {
        buffer_sz = UC_TASK_HEAP - uxTaskGetStackHighWaterMark(ucTask_Handle);
        ESP_LOGI(TAG,"buffer_sz = %d",buffer_sz);
        vTaskDelay(1000/ portTICK_PERIOD_MS);
    }
}

static void uc8151d_task(void *arg)
{
    char imag[5000] = {0};
    screen_init();
    while (1)
    {
        // printf("ok\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
