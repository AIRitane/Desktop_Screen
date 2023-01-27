#include "ssqt.h"
#include "uc8151d.h"
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "page.h"
#include "api.h"
#include "wifi.h"
#include "beep.h"
#include "driver/timer.h"
#include "esp_timer.h"

#define PAGE_TASK_HEAP 20000
#define TAG "PAGE"

static xQueueHandle page_queue = NULL;
TaskHandle_t pageTask_Handle = NULL;
sq_app_t *app;

static void page_task(void *arg);
static void page5_init();
static void page5_show();
static void page6_init();
static void page6_show();
static void page7_init();
static void page7_show();
static void page8_init();
static void page8_show();
//登录
sq_page_t *page1;
static void page1_init()
{
    page1 = creat_page();
    sq_app_page_add(app, page1);
    char str[] = "请链接WiFi登入配置界面\n\nWiFi:Lulincy\n密码:123456780";
    memset(page1->buf,0xff,5000);
    screen_show_chinese(page1->buf, 0, 0, str, 0);
}
static void page1_show()
{
    set_sel_page(app, page1);
    screen_full_display(page1->buf);
}
//检查当前wifi
sq_page_t *page2;
static void page2_init()
{
    page2 = creat_page();
    sq_app_page_add(app, page2);
    char str[] = "当前WiFi不可连接,检查热点是否开启";
    memset(page2->buf,0xff,5000);
    screen_show_chinese(page2->buf, 0, 0, str, 0);
}
static void page2_show()
{
    set_sel_page(app, page2);
    screen_full_display(page2->buf);
}


//检查刚配置wifi
sq_page_t *page3;
static void page3_init()
{
    page3 = creat_page();
    sq_app_page_add(app, page3);
    char str[] = "请检查当前WiFi是否可以正确连接互联网";
    memset(page3->buf,0xff,5000);
    screen_show_chinese(page3->buf, 0, 0, str, 0);
}
static void page3_show()
{
    set_sel_page(app, page3);
    screen_full_display(page3->buf);
}

//主页
sq_page_t *page4;
sq_widget_t* bt_wea_4;
sq_widget_t* one_4;
sq_widget_t* time_4;
sq_widget_t* word_4;
char gImage_1[1152];
uint8_t is_6_first = 0;
uint8_t is_5_first = 0;
void wea_page_4()
{
    page8_init();
    page8_show();
}
void one_page_4()
{
    if (is_5_first==0)
    {
        is_5_first=1;
        page5_init();
    }
    
    page5_show();
}
void time_page_4()
{
    page7_init();
    page7_show();
}
void word_page_4()
{
    if (is_6_first == 0)
    {
        is_6_first = 1;
        page6_init();
    }
    
    page6_show();
}


static void page4_init()
{
    page4 = creat_page();
    sq_app_page_add(app, page4);
    memset(page4->buf,0xff,5000);

    one_4= creat_widget(page4);
    bt_wea_4 = creat_widget(page4);
    word_4 = creat_widget(page4);
    time_4 = creat_widget(page4);
    bt_wea_4 = creat_widget(page4);
    one_4->x=100;
    one_4->y=0;
    one_4->width = 100;
    one_4->hight = 100;
    bt_wea_4->x=0;
    bt_wea_4->y=0;
    bt_wea_4->width = 100;
    bt_wea_4->hight = 100;
    word_4->x=0;
    word_4->y=100;
    word_4->width = 100;
    word_4->hight = 100;
    time_4->x=100;
    time_4->y=100;
    time_4->width = 100;
    time_4->hight = 100;
    // 读取
    FILE *f = fopen("/spiffs/wea.bin", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    fread(gImage_1, 1, 20000, f);
    fclose(f);
    screen_pic_overlay(page4->buf, gImage_1, bt_wea_4->x, bt_wea_4->y, 96, 96);

    f = fopen("/spiffs/one.bin", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    fread(gImage_1, 1, 20000, f);
    fclose(f);
    screen_pic_overlay(page4->buf, gImage_1, one_4->x, one_4->y, 96, 96);

    f = fopen("/spiffs/word.bin", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    fread(gImage_1, 1, 20000, f);
    fclose(f);
    screen_pic_overlay(page4->buf, gImage_1, word_4->x, word_4->y, 96, 96);

    f = fopen("/spiffs/toma.bin", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    fread(gImage_1, 1, 20000, f);
    fclose(f);
    screen_pic_overlay(page4->buf, gImage_1, time_4->x, time_4->y, 96, 96);


    bt_wea_4->clicked = Qsignal();
    time_4->clicked = Qsignal();
    one_4->clicked = Qsignal();
    word_4->clicked = Qsignal();
    
    Qconnect(time_4->clicked, time_page_4);
    Qconnect(bt_wea_4->clicked, wea_page_4);
    Qconnect(one_4->clicked, one_page_4);
    Qconnect(word_4->clicked, word_page_4);
}
static void page4_show()
{
    set_sel_page(app, page4);
    screen_full_display(page4->buf);
}

//一言
sq_page_t *page5;
sq_widget_t* bt_one_5;
sq_widget_t* bt_re_5;
char *one_5;
char one_title[] = "Hitokoto";
void fresh_5()
{
    one_5 = get_api_one();
    printf("%s\n",one_5);
    screen_draw_fill(page5->buf, 0, 40,199, 199, 1);
    screen_show_chinese(page5->buf, 0, 40, one_5, 0);
    // screen_set_ram(page5->buf);
    // screen_partial_display(0,0, page5->buf,200, 200);
    // refresh_part();
    page5_show();
}
void return_page_4()
{
    page4_show();
}
static void page5_init()
{
    page5 = creat_page();
    sq_app_page_add(app, page5);
    memset(page5->buf,0xff,5000);

    bt_re_5 = creat_widget(page5);
    bt_re_5->x=0;
    bt_re_5->y=0;
    bt_re_5->width = 30;
    bt_re_5->hight = 30;
    screen_draw_fill(page5->buf, 0, 0,30, 30, 0);

    bt_one_5 = creat_widget(page5);
    bt_one_5->x=30;
    bt_one_5->y=30;
    bt_one_5->width = 170;
    bt_one_5->hight = 170;
    send_api_event(API_ONE);
    one_5 = get_api_one();
    printf("%s\n",one_5);
    screen_show_chinese(page5->buf, 50, 0, one_title, 0);

    bt_re_5->clicked = Qsignal();
    Qconnect(bt_re_5->clicked, return_page_4);
    bt_one_5->clicked = Qsignal();
    Qconnect(bt_one_5->clicked, fresh_5);
}
static void page5_show()
{
    set_sel_page(app, page5);
    screen_full_display(page5->buf);
}

//单词
sq_page_t *page6;
sq_widget_t* bt_word_6;
sq_widget_t* bt_re_6;
char *word_6;
int32_t word_index=0;
char en_word_6[20];
char word_last_6[200];

void left_swipe_6()
{
    memset(word_last_6,0,sizeof(word_last_6));
    memcpy(word_last_6,word_6,strlen(word_6)+1);
    printf("left\n");
    memset(page6->buf,0xff,5000);
    word_index--;
    if (word_index<1)
    {
        word_index=1;
    }

    FILE *f = fopen("/spiffs/word.txt", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    for (uint32_t i = 0; i < word_index-1; i++)
        fscanf(f,"%*[^\n]%*c");
    fscanf(f,"%s",en_word_6);
    fclose(f);
    printf("%s\n",en_word_6);
    screen_show_chinese(page6->buf, 0, 35, en_word_6, 0);
    set_api_word(en_word_6);
    send_api_event(API_WORD);
    word_6 = get_api_word();
    while (strcmp(word_last_6, word_6) == 0)
    {
        word_6 = get_api_word();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    printf("%s\n",word_6);
    screen_show_chinese(page6->buf, 0, 57, word_6, 0);
    
    screen_draw_fill(page6->buf, 0, 0,30, 30, 0);
    page6_show();
    // screen_set_ram(page6->buf);
    // screen_partial_display(0,0, page6->buf,200, 200);
    // refresh_part();
}
void right_swipe_6()
{
    memset(word_last_6,0,sizeof(word_last_6));
    memcpy(word_last_6,word_6,strlen(word_6)+1);
    printf("right\n");
    memset(page6->buf,0xff,5000);
    word_index++;
    FILE *f = fopen("/spiffs/word.txt", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    for (uint32_t i = 0; i < word_index; i++)
        fscanf(f,"%*[^\n]%*c");
    fscanf(f,"%s",en_word_6);
    fclose(f);
    printf("%s\n",en_word_6);
    screen_show_chinese(page6->buf, 0, 35, en_word_6, 0);
    set_api_word(en_word_6);
    send_api_event(API_WORD);
    word_6 = get_api_word();
    while (strcmp(word_last_6, word_6) == 0)
    {
        word_6 = get_api_word();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    printf("%s\n",word_6);
    screen_show_chinese(page6->buf, 0, 57, word_6, 0);

    screen_draw_fill(page6->buf, 0, 0,30, 30, 0);
    page6_show();
    // screen_set_ram(page6->buf);
    // screen_partial_display(0,0, page6->buf,200, 200);
    // refresh_part();
}
static void page6_init()
{
    page6 = creat_page();
    sq_app_page_add(app, page6);
    memset(page6->buf,0xff,5000);

    bt_re_6 = creat_widget(page6);
    bt_re_6->x=0;
    bt_re_6->y=0;
    bt_re_6->width = 30;
    bt_re_6->hight = 30;
    screen_draw_fill(page6->buf, 0, 0,30, 30, 0);

    bt_word_6 = creat_widget(page6);
    bt_word_6->x=0;
    bt_word_6->y=30;
    bt_word_6->width = 200;
    bt_word_6->hight = 170;

    set_api_word("abandon");
    send_api_event(API_WORD);
    word_6 = get_api_word();
    while (strlen(word_6) == 0)
    {
        word_6 = get_api_word();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    printf("%s\n",word_6);printf("%d\n",strlen(word_6));
    screen_show_chinese(page6->buf, 0, 35, "abandon", 0);
    screen_show_chinese(page6->buf, 0, 57, word_6, 0);

    bt_re_6->clicked = Qsignal();
    Qconnect(bt_re_6->clicked, return_page_4);
    bt_word_6->left_swipe = Qsignal();
    bt_word_6->right_swipe = Qsignal();


    Qconnect(bt_word_6->left_swipe, left_swipe_6);
    Qconnect(bt_word_6->right_swipe, right_swipe_6);
}
static void page6_show()
{
    set_sel_page(app, page6);
    screen_full_display(page6->buf);
}

//天气
//80*80
sq_page_t *page7;
sq_widget_t* bt_re_7;
sq_widget_t* bt_start_7;
uint8_t is_work_7 = 0;
uint32_t work_time_7,rest_time_7,timer_wk_7;
char c_work_7[20]= {0};
static void page7_fresh()
{
    memset(page7->buf,0xff,5000);
    if (is_work_7)
    {
        work_time_7 --;
        memset(c_work_7,0,sizeof(c_work_7));
        sprintf(c_work_7, "Working Time\n %d", work_time_7);
        if (work_time_7 == 0)
        {
            is_work_7 = 0;
            work_time_7 = user_info.toma_wt;
            send_beep_event(BEEP_LONG);
            send_beep_event(BEEP_LONG);
            memset(c_work_7,0,sizeof(c_work_7));
            sprintf(c_work_7, "Rest Time\n %d", rest_time_7);
        }
        
    }
    else
    {
        rest_time_7--;
        memset(c_work_7,0,sizeof(c_work_7));
        sprintf(c_work_7, "Rest Time\n %d", rest_time_7);
        if (rest_time_7 == 0)
        {
            is_work_7 = 1;
            rest_time_7 = user_info.toma_rt;
            send_beep_event(BEEP_LONG);
            send_beep_event(BEEP_LONG);
            memset(c_work_7,0,sizeof(c_work_7));
            sprintf(c_work_7, "Working Time\n %d", work_time_7);
        }
    }
    
    screen_show_chinese(page7->buf, 40, 0, "Tomato Time", 0);
    screen_draw_fill(page7->buf, 0, 0,30, 30, 0);
    if (work_time_7==1)
    {
        screen_show_chinese(page7->buf, 10, 40, c_work_7, 0);
    }else
    {
        screen_show_chinese(page7->buf, 35, 40, c_work_7, 0);
    }
    
    
    page7_show();
    // screen_set_ram(page7->buf);
    // screen_partial_display(0,0, page7->buf,200, 200);
    // refresh_part();
}

esp_timer_handle_t esp_timer_handle;

void page7_timer_callback(void *arg)
{
    page7_fresh();
}

static void page7_timer_init()
{
    esp_timer_create_args_t fw_timer = {
        .callback = &page7_timer_callback, // 定时器回调函数
        .arg = NULL,
        .name = "page7_timer", // 定时器名称
    };

    esp_err_t err = esp_timer_create(&fw_timer, &esp_timer_handle);
    err = esp_timer_start_periodic(esp_timer_handle, 1000 * 1000*60); // us级定时，1000*1000=1s
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "TP定时器初始化错误");
    }
    esp_timer_stop(esp_timer_handle);
    timer_wk_7 = 0;
}

void re_page_4_7()
{
    //停止计时器
    esp_timer_stop(esp_timer_handle);
    esp_timer_delete(esp_timer_handle);
    //清空计时器值
    work_time_7 = user_info.toma_wt;
    rest_time_7 = user_info.toma_rt;
    is_work_7 = 1;
    return_page_4();
}

void start_timer_7()
{
    printf("tomato ok\n");
    if (timer_wk_7==0)
    {
        esp_timer_start_periodic(esp_timer_handle, 1000 * 1000*60);
        timer_wk_7 = 1;
    }
    else
    {
        esp_timer_stop(esp_timer_handle);
        timer_wk_7 = 0;
    }
    if (is_work_7 == 1)
    {
        work_time_7++;
    }
    else
    {
        rest_time_7++;
    }
    
    
    page7_fresh();
}  

uint8_t is_first_in = 0;
static void page7_init()
{
    if (is_first_in==0)
    {
        is_first_in=1;
        beep_init();
        page7 = creat_page();
        sq_app_page_add(app, page7);
        
        bt_re_7 = creat_widget(page7);
        bt_start_7 = creat_widget(page7);
        bt_re_7->clicked = Qsignal();
        bt_start_7->clicked = Qsignal();
        Qconnect(bt_re_7->clicked, re_page_4_7);
        Qconnect(bt_start_7->clicked, start_timer_7);
    }
    memset(page7->buf,0xff,5000);
    screen_show_chinese(page7->buf, 40, 0, "Tomato Time", 0);

    //按键点击开始
    is_work_7 = 1;
    
    memset(c_work_7,0,sizeof(c_work_7));
    sprintf(c_work_7, "Working Time\n %d", user_info.toma_wt);
    work_time_7 = user_info.toma_wt;
    rest_time_7 = user_info.toma_rt;
    screen_show_chinese(page7->buf, 10, 40, c_work_7, 0);
    bt_re_7->x=0;
    bt_re_7->y=0;
    bt_re_7->width = 30;
    bt_re_7->hight = 30;
    bt_start_7->x=0;
    bt_start_7->y=31;
    bt_start_7->width = 200;
    bt_start_7->hight = 170;
    screen_draw_fill(page7->buf, 0, 0,30, 30, 0);
    screen_show_chinese(page7->buf, 40, 150, "点击开始", 0);
    page7_timer_init();
}
static void page7_show()
{
    set_sel_page(app, page7);
    screen_full_display(page7->buf);
}

sq_page_t *page8;
sq_widget_t* bt_wea_8;
sq_widget_t* bt_re_8;
yiketianqi_t* weather;
uint8_t is_first_in_8 = 0;
void fresh_8()
{
    one_5 = get_api_one();
    printf("%s\n",one_5);
    screen_draw_fill(page5->buf, 0, 40,199, 199, 1);
    screen_show_chinese(page5->buf, 0, 40, one_5, 0);
    // screen_set_ram(page5->buf);
    // screen_partial_display(0,0, page5->buf,200, 200);
    // refresh_part();
    page8_show();
}
static void page8_init()
{
    if (is_first_in_8==0)
    {
        is_first_in_8=1;
     page8 = creat_page();
    sq_app_page_add(app, page8);
    bt_re_8 = creat_widget(page8);
    bt_wea_8 = creat_widget(page8);
        bt_re_8->clicked = Qsignal();
    Qconnect(bt_re_8->clicked, return_page_4);
    }
    
   
    memset(page8->buf,0xff,5000);

    bt_re_8->x=0;
    bt_re_8->y=0;
    bt_re_8->width = 30;
    bt_re_8->hight = 30;
    screen_draw_fill(page8->buf, 0, 0,30, 30, 0);

    bt_wea_8->x=0;
    bt_wea_8->y=30;
    bt_wea_8->width = 200;
    bt_wea_8->hight = 170;
    set_api_local(user_info.addr);
    send_api_event(API_WEATHER);
    weather = get_api_weather();
    while (strlen(weather->win_speed) == 0)
    {
        weather = get_api_weather();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    printf("%s\n",weather->win_speed);
    char str1[200];
    sprintf(str1,"%s,%s,气温:%s\n气温最高/最低:%s/%s\n%s,%s,%s\nair:%s,pres:%s,humu:%s",weather->city,weather->wea,weather->tem,weather->tem_day,weather->tem_night,weather->win,weather->win_speed,weather->win_meter,weather->air,weather->pressure,weather->humidity);
    printf("%s\n",str1);
    screen_show_chinese(page8->buf, 0, 35, str1, 0);

    // bt_one_5->clicked = Qsignal();
    // Qconnect(bt_one_5->clicked, fresh_5);
}
static void page8_show()
{
    set_sel_page(app, page8);
    screen_full_display(page8->buf);
}


void page_init()
{
    screen_init();
    app = sq_creat_app();
    ssqt_init(app);
    api_init();

    page1_init();
    page2_init();
    page3_init();
    page4_init();

    page_queue = xQueueCreate(10, sizeof(uint32_t));
    // start gpio task
    xTaskCreatePinnedToCore(page_task, "page_task", PAGE_TASK_HEAP, NULL, 6, pageTask_Handle, 1);
}

static void page_task(void *arg)
{
    uint32_t evt;
    uint32_t buffer_sz;
    while (1)
    {
        buffer_sz = uxTaskGetStackHighWaterMark(pageTask_Handle);
        ESP_LOGW(TAG,"page堆栈剩余大小 = %d\n", buffer_sz);
        if (xQueueReceive(page_queue, &evt, portMAX_DELAY))
        {
            switch (evt)
            {
            case PAGE_1:
                page1_show();
                break;
                case PAGE_2:
                page2_show();
                break;
                case PAGE_3:
                page3_show();
                break;
                case PAGE_4:
                page4_show();
                break;
                case PAGE_5:
                page5_show();
                break;
            
            default:
                break;
            }
            
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
}

void send_page_event(page_index_e type)
{
    uint32_t evt = type;
    xQueueSend(page_queue, &evt, 0);
}

void send_page_event_from_isr(page_index_e type)
{
    uint32_t evt = type;
    xQueueSendFromISR(page_queue, &evt, 0);
}