#include "ssqt.h"
#include <stdio.h>
#include <string.h>
#include "driver/timer.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "tp.h"

#define TAG "SSQT"

static xQueueHandle touch_queue = NULL;
TaskHandle_t touchTask_Handle = NULL;
#define TOUCH_TASK_HEAP 2048

esp_timer_handle_t esp_timer_handle;
TouchPoint_T *touch;

/**********************************ui数据结构*********************************************/
sq_page_t *creat_page()
{
    sq_page_t *page = (sq_page_t *)malloc(sizeof(sq_page_t));
    page->widget = (sq_node_t *)malloc(sizeof(sq_node_t));
    page->widget->next = NULL;

    return page;
}

sq_widget_t *creat_widget(sq_page_t *page)
{
    sq_widget_t *widget = (sq_widget_t *)malloc(sizeof(sq_widget_t));
    sq_node_t *node = (sq_node_t *)malloc(sizeof(sq_node_t));
    node->data = widget;
    node->next = NULL;
    sq_node_t *p;
    p = page->widget;
    while (p->next != NULL)
    {
        p = p->next;
    }

    p->next = node;
    return widget;
}

void delete_page(sq_page_t *page)
{
    sq_node_t *p, *q;
    p = page->widget;

    while (p->next != NULL)
    {
        q = p;
        p = p->next;
        free(q->data);
        free(q);
    }
    free(p->data);
    free(p);
    free(page);
}

void delete_widget(sq_page_t *page, sq_widget_t *widget)
{
    sq_node_t *p, *q;
    p = page->widget;
    q = p;
    while (p->next != NULL)
    {
        if (p->next->data == widget)
        {
            q->next = p->next->next;
            free(p->data);
            free(p);
            break;
        }
        q = p;
        p = p->next;
    }
}

sq_app_t *sq_creat_app()
{
    sq_app_t *app = (sq_app_t *)malloc(sizeof(sq_app_t));
    app->page = (sq_pnode_t *)malloc(sizeof(sq_pnode_t));
    app->page->next = NULL;
    app->sel_page = NULL;
    return app;
}

void sq_app_page_add(sq_app_t *app, sq_page_t *page)
{
    sq_pnode_t *node = (sq_pnode_t *)malloc(sizeof(sq_pnode_t));
    node->data = page;
    node->next = NULL;
    sq_pnode_t *p;
    p = app->page;
    while (p->next != NULL)
    {
        p = p->next;
    }

    p->next = node;
}

void delete_app_page(sq_app_t *app, sq_page_t *page)
{
    sq_pnode_t *p, *q;
    p = app->page;
    q = p;
    while (p->next != NULL)
    {
        if (p->next->data == page)
        {
            q->next = p->next->next;
            free(p);
            break;
        }
        q = p;
        p = p->next;
    }
}

void set_sel_page(sq_app_t *app, sq_page_t *page)
{
    sq_pnode_t *p;
    p = app->page;
    while (p->next != NULL)
    {
        if (p->next->data == page)
        {
            app->sel_page = p->next;
            break;
        }
        p = p->next;
    }
}

/**********************************ui定时器监控*********************************************/
// esp_timer_handle_t esp_timer_handle;
// TouchPoint_T *touch;
void ui_timer_callback(void *arg)
{
    touch = get_touch_point();
    if (touch->geste != TP_NONE)
    {
        uint32_t evt = touch->geste;
        xQueueSendFromISR(touch_queue, &evt, 0);
    }
}

static void ui_timer_init()
{
    esp_timer_create_args_t fw_timer = {
        .callback = &ui_timer_callback, // 定时器回调函数
        .arg = NULL,
        .name = "ui_timer", // 定时器名称
    };

    esp_err_t err = esp_timer_create(&fw_timer, &esp_timer_handle);
    err = esp_timer_start_periodic(esp_timer_handle, 1000 * 1); // us级定时，1000*1000=1s
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "ui定时器初始化错误");
    }
}

/***************************************ui任务*********************************************/
// static xQueueHandle touch_queue = NULL;
// TaskHandle_t touchTask_Handle = NULL;
// #define TOUCH_TASK_HEAP 2048
static void touch_task(void *arg)
{
    sq_app_t *app = (sq_app_t *)arg;
    uint32_t evt;
    // int buffer_sz = 0;
    sq_widget_t *sel_widge = NULL;
    while (1)
    {
        sel_widge = NULL;
        // buffer_sz = uxTaskGetStackHighWaterMark(touchTask_Handle);
        // ESP_LOGW(TAG, "touch堆栈剩余大小 = %d\n", buffer_sz);
        if (xQueueReceive(touch_queue, &evt, portMAX_DELAY))
        {
            if (app == NULL)
            {
                ESP_LOGE(TAG, "APP指针为空");
                continue;
            }
            if (app->sel_page == NULL)
            {
                continue;
            }

            // 查找被点击的组件
            sq_node_t *p;
            sq_widget_t *q;
            p = app->sel_page->data->widget;
            while (p->next != NULL)
            {
                q = p->next->data;
                if (q->x <= touch->last_x && touch->last_x <= (q->x + q->width) && q->y <= touch->last_y && touch->last_y <= (q->y + q->hight))
                {
                    if (sel_widge == NULL)
                    {
                        sel_widge = q;
                    }
                    else
                    {
                        if (sel_widge->width * sel_widge->hight >= q->width * q->hight)
                            sel_widge = q;
                    }
                }
                p = p->next;
            }
            if (sel_widge == NULL)
            {
                // ESP_LOGE(TAG,"sel_widge指针为空");
                continue;
            }

            // 判断组件的事件
            geste_e *event = (geste_e *)(&evt);
            switch (*event)
            {
            case TP_UP:
                emit(sel_widge->up_swipe);
                break;
            case TP_DOWEN:
                emit(sel_widge->down_swipe);
                break;
            case TP_LEFT:
                emit(sel_widge->left_swipe);
                break;
            case TP_RIGHT:
                emit(sel_widge->right_swipe);
                break;
            case TP_CLICKED:
                emit(sel_widge->clicked);
                break;
            case TP_LONG_CLICKED:
                emit(sel_widge->long_clicked);
                break;
            default:
                break;
            }
        }
    }
}

void ssqt_init(sq_app_t *app)
{
    screen_tp_init();
    ui_timer_init();
    touch_queue = xQueueCreate(20, sizeof(uint32_t));
    xTaskCreatePinnedToCore(touch_task, "touch_task", TOUCH_TASK_HEAP, (void *)app, 3, touchTask_Handle, 1);
}