/*
 * wifi.c
 *
 *  Created on: 2021年7月5日
 *      Author: jenson
 */
#include <sys/param.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include <string.h>
#include "esp_spiffs.h"
#include "esp_http_server.h"
#include "wifi.h"

#define WIFI_TASK_HEAP 20000
#define WIFI_CONNECT_MAXIMUM_RETRY 10

#define TAG "WIFI"

typedef enum
{
    WIFI_CONNECTED,
    WIFI_DISCONNECTED,
    WIFI_USER_LOG_SUCEESS,
    WIFI_NO_PASSWORLD,
    WIFI_HAS_PASSWORLD,
} wifi_event_e;

// 连接尝试次数
static int s_retry_num = 0;
static EventGroupHandle_t login_group;
TaskHandle_t wifiTask_Handle = NULL;
user_info_t user_info;

void wifi_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);
void wifi_event_task(void *params);

void nvs_save_wifi_info(char *key, void *val, uint8_t mode)
{
    esp_err_t err;
    nvs_handle_t nvs_handle;
    err = nvs_open("wificonfig", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }
    if (mode == 0)
    {
        ESP_ERROR_CHECK(nvs_set_u8(nvs_handle, key, *(uint8_t *)val));
    }
    else if (mode == 1)
    {
        ESP_ERROR_CHECK(nvs_set_str(nvs_handle, key, (char *)val));
    }
    else if (mode == 2)
    {
        ESP_ERROR_CHECK(nvs_set_u32(nvs_handle, key, *(uint32_t *)val));
    }

    ESP_ERROR_CHECK(nvs_commit(nvs_handle));
    nvs_close(nvs_handle);
}

NVS_WIFI_INFO_E nvs_read_wifi_info(char **ssid, char **psw,char *city,uint32_t *toma_wt,uint32_t* toma_rt)
{
    esp_err_t err;
    nvs_handle_t nvs_handle;
    err = nvs_open("wificonfig", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return NVS_WIFI_INFO_ERROR;
    }

    size_t ssid_len = 32;
    size_t psw_len = 32;
    *ssid = (char *)malloc(ssid_len);
    *psw = (char *)malloc(psw_len);

    uint8_t wifi_flag = 0;
    size_t len=20;
    nvs_get_u8(nvs_handle, "wifi_flag", &wifi_flag);
    (nvs_get_str(nvs_handle, "ssid", *ssid, &ssid_len));
    (nvs_get_str(nvs_handle, "psw", *psw, &psw_len));
    (nvs_get_str(nvs_handle, "city", city, &len));
    (nvs_get_u32(nvs_handle, "toma_wt", toma_wt));
    (nvs_get_u32(nvs_handle, "toma_rt", toma_rt));
    nvs_close(nvs_handle);
    if (wifi_flag == NVS_WIFI_INFO_SAVE)
    {
        return NVS_WIFI_INFO_SAVE;
    }
    else
    {
        return NVS_WIFI_INFO_NULL;
    }
}

void nvs_init()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

wifi_config_t wifi_cfg;
void wifi_start(wifi_t *wifi)
{
    login_group = xQueueCreate(10, sizeof(uint32_t));
    nvs_init();
    uint32_t wifi_nvs_cofig = nvs_read_wifi_info(&(wifi->ssid), &(wifi->password),user_info.addr,&user_info.toma_wt,&user_info.toma_rt);
    if (wifi_nvs_cofig != NVS_WIFI_INFO_SAVE)
    {
        uint32_t evt = WIFI_NO_PASSWORLD;
        xQueueSend(login_group, &evt, 0);
    }
    else
    {
        uint32_t evt = WIFI_HAS_PASSWORLD;
        xQueueSend(login_group, &evt, 0);
    }

    // 初始化IP协议层
    ESP_ERROR_CHECK(esp_netif_init());
    // 启动事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // 创建Station模式的IP协议层
    wifi->netif = esp_netif_create_default_wifi_sta();
    wifi->ap = esp_netif_create_default_wifi_ap();

    // 获取WiFi默认初始化配置
    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));

    // 注册WiFi事件
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    // 注册WiFi连接事件
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    // 注册IP事件，Station工作模式下获取IPV4事件
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    // 注册IP事件，Station工作模式下获取IPV6事件
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &wifi_event_handler, NULL));

    // 将wifi_config_t对象所有字段清零
    memset(&wifi_cfg, 0, sizeof(wifi_config_t));

    // 复制SSID到配置对象中，SSID长度为33个字符
    // 如果SSID过长，将截断，将导致无法连接上WiFi网络
    size_t ssid_len = strlen(wifi->ssid);
    if (ssid_len > 33)
    {
        ssid_len = 33;
    }
    memcpy(wifi_cfg.sta.ssid, wifi->ssid, ssid_len);

    // 复制密码
    size_t pass_len = strlen(wifi->password);
    if (pass_len > 64)
    {
        pass_len = 64;
    }
    memcpy(wifi_cfg.sta.password, wifi->password, pass_len);

    // ESP_LOGI(TAG, "wifi ssid:%s,passwrod:%s\n", wifi_cfg.sta.ssid, wifi_cfg.sta.password);
    // 设置认证模式
    if (pass_len > 0)
    {
        wifi_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }
    else
    {
        wifi_cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;
    }
    // 保护管理帧的配置
    wifi_cfg.sta.pmf_cfg.capable = true;
    wifi_cfg.sta.pmf_cfg.required = false;

    wifi_config_t ap_config = {
        .ap = {
            .ssid = AP_WIFI_SSID,
            .ssid_len = strlen(AP_WIFI_PASS),
            .channel = 11,
            .password = AP_WIFI_PASS,
            .max_connection = AP_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };

    // 设置WiFi的工作模式为Station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    // 应用WiFi配置
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
    // 启动WiFi连接
    ESP_ERROR_CHECK(esp_wifi_start());

    // 创建WiFi事件处理任务
    xTaskCreatePinnedToCore(wifi_event_task, "wifi_event_task", 10000, (void *)wifi, 3, wifiTask_Handle, 0);
}

void wifi_stop(wifi_t *wifi)
{
    esp_wifi_stop();
}

/**
 * @brief WiFi事件处理函数
 * */
void wifi_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    // WiFi在Station模式启动事件
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    // WiFi在Station模式下断开连接
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < WIFI_CONNECT_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            // 触发WiFi断开事件
            uint32_t evt;
            evt = WIFI_DISCONNECTED;
            xQueueSendFromISR(login_group, &evt, NULL);
        }
    }

    // WiFi连接后，获取IP事件
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        // 触发事件WIFI_CONNECTED组
        uint32_t evt;
        evt = WIFI_CONNECTED;
        xQueueSendFromISR(login_group, &evt, NULL);
    }
}

/********************************************用户配置网页相关设置***************************************************/
char _html[3000] = {0};

void get_html_buf()
{
    // esp_vfs_spiffs_conf_t conf = {
    //     .base_path = "/spiffs",
    //     .partition_label = NULL,
    //     .max_files = 5,
    //     .format_if_mount_failed = false};

    // // 挂载注册
    // esp_err_t ret = esp_vfs_spiffs_register(&conf);
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "磁盘挂载失败");
    //     return;
    // }
    // 读取
    FILE *f = fopen("/spiffs/login.html", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    fread(_html, 1, 20000, f);
    fclose(f);
}

// get方法推送HTML主页文件
static esp_err_t gone_get_handler(httpd_req_t *req)
{
    const char *resp_str = (const char *)req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}
static const httpd_uri_t gone = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = gone_get_handler,
    .user_ctx = _html,
};

// 查找匹配函数
static int find_str(char source[], char searchfor[])
{
    int k = 0;
    for (int i = 0; source[i] != '\0'; i++)
    {
        if (k == 0 && source[i] == searchfor[k])
        {
            k++;
            continue;
        }
        if (k > 0 && source[i] == searchfor[k])
        {
            k++;
            if (k == strlen(searchfor))
            {
                return i - k + 1; // 返回目标字符串的子串末尾-子串长度+1
            }
            continue;
        }
        k = 0;
    }
    return -1; // 没找到返回-1
}

void uzip_user_info(char *str, user_info_t *user_info)
{
    int index, index1;
    index = find_str(str, "username");
    index1 = find_str(str, "password");
    memcpy(user_info->ssid, str + index + 9, index1 - index - 10);
    index = find_str(str, "password");
    index1 = find_str(str, "address");
    memcpy(user_info->psw, str + index + 9, index1 - index - 10);
    index = find_str(str, "address");
    index1 = find_str(str, "tomato_s");
    memcpy(user_info->addr, str + index + 8, index1 - index - 9);
    index = find_str(str, "tomato_s");
    user_info->toma_wt = (*(str + index + 9) - 48) * 600 + (*(str + index + 10) - 48) * 60 + (*(str + index + 14) - 48) * 10 + (*(str + index + 15) - 48);
    index = find_str(str, "tomato_e");
    user_info->toma_rt = (*(str + index + 9) - 48) * 600 + (*(str + index + 10) - 48) * 60 + (*(str + index + 14) - 48) * 10 + (*(str + index + 15) - 48);
}

// pose方法取回用户配置信息
static esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[200];
    int ret, remaining = req->content_len;
    if (remaining > 200)
    {
        ESP_LOGE(TAG, "POSE返回的数据太长，超过缓存区大小");
        return ESP_FAIL;
    }

    while (remaining > 0)
    {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                                  MIN(remaining, sizeof(buf)))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;
    }
    uzip_user_info(buf, &user_info);
    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    uint32_t evt;
    evt = WIFI_USER_LOG_SUCEESS;
    xQueueSendFromISR(login_group, &evt, NULL);
    return ESP_OK;
}
static const httpd_uri_t echo = {
    .uri = "/",
    .method = HTTP_POST,
    .handler = echo_post_handler,
    .user_ctx = NULL};

static void set_net_ip()
{
    tcpip_adapter_init();
    tcpip_adapter_ip_info_t ip_info = {
        .ip.addr = ipaddr_addr(CONNECT_HTTP_IP),
        .netmask.addr = ipaddr_addr(CONNECT_HTTP_MASK),
        .gw.addr = ipaddr_addr(CONNECT_HTTP_GW),
    };
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info));
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));

    ESP_LOGI(TAG, "ip 值重新配置为:" CONNECT_HTTP_IP "");
}

static httpd_handle_t start_webserver(void)
{
    get_html_buf();
    set_net_ip();
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &gone);
        httpd_register_uri_handler(server, &echo);
        return server;
    }
    return NULL;
}

void wifi_event_task(void *params)
{
    wifi_t *wifi = (wifi_t *)params;
    int buffer_sz = 0;
    uint32_t evt;
    start_webserver();
    for (;;)
    {
        buffer_sz = WIFI_TASK_HEAP - uxTaskGetStackHighWaterMark(wifiTask_Handle);
        ESP_LOGW(TAG, "wifi堆栈剩余大小 = %d\n", buffer_sz);
        if (buffer_sz <= 400)
        {
            ESP_LOGW(TAG, "wifi堆栈剩余大小 = %d\n", buffer_sz);
        }

        if (xQueueReceive(login_group, &evt, portMAX_DELAY))
        {
            if (evt == WIFI_USER_LOG_SUCEESS)
            {
                if (wifi->callback != NULL)
                {
                    wifi->callback(EVENT_USER_LOGIN);
                }
                uint8_t val;
                val = NVS_WIFI_INFO_SAVE;
                nvs_save_wifi_info("wifi_flag", &val, 0);
                nvs_save_wifi_info("ssid", user_info.ssid, 1);
                nvs_save_wifi_info("psw", user_info.psw, 1);
                nvs_save_wifi_info("city", user_info.addr, 1);
                nvs_save_wifi_info("toma_wt", &user_info.toma_wt, 2);
                nvs_save_wifi_info("toma_rt", &user_info.toma_rt, 2);

                size_t ssid_len = strlen(user_info.ssid);
                memcpy(wifi_cfg.sta.ssid, user_info.ssid, ssid_len);
                size_t pass_len = strlen(user_info.psw);
                memcpy(wifi_cfg.sta.password, user_info.psw, pass_len);
                esp_wifi_stop();
                esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg);
                s_retry_num = 0;
                esp_wifi_start();
                esp_wifi_connect();
            }
            if (evt == WIFI_NO_PASSWORLD)
            {
                if (wifi->callback != NULL)
                {
                    wifi->callback(EVENT_USER_ONNVS);
                }
            }
            if (evt == WIFI_HAS_PASSWORLD)
            {
                if (wifi->callback != NULL)
                {
                    wifi->callback(EVENT_USER_HASNVS);
                }
            }

            // WiFi连接并分配IP事件
            if (evt == WIFI_CONNECTED)
            {
                tcpip_adapter_ip_info_t ip_info;
                // 查询、转换IP地址、网关、子网掩码
                ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));

                // 自定义WiFi事件回调函数
                if (wifi->callback != NULL)
                {
                    // ESP_LOGI(TAG, "connected,call callback function");
                    wifi->callback(EVENT_GOT_IP);
                }
                else
                {
                    ESP_LOGI(TAG, "callback is null");
                }
            }

            // WiFi断开连接事件 portMAX_DELAY
            if (evt == WIFI_DISCONNECTED)
            {
                ESP_LOGI(TAG, "waiting for wifi disconnect");
                ESP_LOGI(TAG, "wifi disconected from the wifi network");

                // 自定义WiFi事件回调函数
                if (wifi->callback != NULL)
                {
                    ESP_LOGI(TAG, "disconnected,call callback function");
                    wifi->callback(EVENT_DISCONNECT);
                }
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

