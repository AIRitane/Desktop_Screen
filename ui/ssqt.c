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
    return app;
}

void sq_app_page_add(sq_app_t * app,sq_page_t * page)
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

void delete_app_page(sq_app_t * app,sq_page_t * page)
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

/***************************************ui任务*********************************************/
static xQueueHandle touch_queue = NULL;
TaskHandle_t touchTask_Handle = NULL;
static void touch_task(void *arg)
{
    uint32_t evt;
    int buffer_sz = 0;
    while (1)
    {
        buffer_sz = uxTaskGetStackHighWaterMark(touchTask_Handle);
        ESP_LOGW(TAG,"beep堆栈剩余大小 = %d\n", buffer_sz);
        if(xQueueReceive(touch_queue, &evt, portMAX_DELAY))
        {
            //处理函数
            //是那个组件被点击
            //判断该组件的该事件是否为空，是就记录，不断轮训记录，记录为空直接跳过，不为空弹出事件
            //在事件内部控制刷新方式
        }
    }
}

/**********************************ui定时器监控*********************************************/
esp_timer_handle_t esp_timer_handle;
TouchPoint_T* touch;
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
			.callback = &ui_timer_callback, //定时器回调函数
			.arg = NULL,
			.name = "ui_timer", //定时器名称
	};
 
	esp_err_t err = esp_timer_create(&fw_timer,&esp_timer_handle);
	err = esp_timer_start_periodic(esp_timer_handle,1000*1); //us级定时，1000*1000=1s
	if(err!=ESP_OK){
		ESP_LOGE(TAG,"ui定时器初始化错误");
	}
}
