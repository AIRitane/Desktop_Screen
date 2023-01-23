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

sq_app_t *app;
sq_page_t * page_1;
sq_widget_t *button;
void clicked1()
{
    printf("1：我被点击了\n");
}
void clicked2()
{
    printf("2：我被点击了\n");
}
void long_clicked()
{
    printf("我被长按了\n");
}
void app_main(void)
{
    beep_init();

    app  = sq_creat_app();
    page_1 = creat_page();
    sq_app_page_add(app, page_1);
    button = creat_widget(page_1);
    button->x = 100;
    button->y = 100;
    button->width = 50;
    button->hight = 50;
    button->clicked = Qsignal();
    button->long_clicked = Qsignal();
    Qconnect(button->clicked, clicked1);
    Qconnect(button->clicked, clicked2);
    Qconnect(button->long_clicked, long_clicked);
    set_sel_page(app,page_1);
    ssqt_init(app);
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
