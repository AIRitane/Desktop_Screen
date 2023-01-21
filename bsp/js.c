#include "js.h"
#include "cJSON.h"
#include "esp_log.h"
#include <string.h>

#define TAG "JS"

/******************************************工具函数解析********************************************/
//查找匹配函数
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

/******************************************一言js解析********************************************/
void one_js(char *str,char *buf)
{
    char *ret;
    ret = strchr(str, '{');
    cJSON *pJsonRoot = cJSON_Parse(ret);
    if (pJsonRoot != NULL)
    {
        cJSON *pHitoAdress = cJSON_GetObjectItem(pJsonRoot, "hitokoto"); // 解析c字段字符串内容
        if (!pHitoAdress)
            return; // 判断字段是否json格式
        else
        {
            if (cJSON_IsString(pHitoAdress)) // 判断mac字段是否string类型
            {
                strcpy(buf, pHitoAdress->valuestring); // 拷贝内容到字符串数组
            }
            else
            {
                ESP_LOGE(TAG, "解析JS失败");
            }
        }
    }
    else
    {
        ESP_LOGE(TAG, "解析JS失败");
    }
}

/******************************************天气js解析********************************************/
void yiketianqi_js(char *str,yiketianqi_t *buf)
{
    char *ret;
    ret = strchr(str, '{');
    cJSON *pJsonRoot = cJSON_Parse(ret);
    if (pJsonRoot != NULL)
    {
        cJSON *pCityAdress = cJSON_GetObjectItem(pJsonRoot, "city"); // 解析c字段字符串内容
        cJSON *pWeaAdress = cJSON_GetObjectItem(pJsonRoot, "wea"); // 解析c字段字符串内容
        cJSON *pWea_imgAdress = cJSON_GetObjectItem(pJsonRoot, "wea_img"); // 解析c字段字符串内容
        cJSON *pTemAdress = cJSON_GetObjectItem(pJsonRoot, "tem"); // 解析c字段字符串内容
        cJSON *pTem_dayAdress = cJSON_GetObjectItem(pJsonRoot, "tem_day"); // 解析c字段字符串内容
        cJSON *pTem_nightAdress = cJSON_GetObjectItem(pJsonRoot, "tem_night"); // 解析c字段字符串内容
        cJSON *pWinAdress = cJSON_GetObjectItem(pJsonRoot, "win"); // 解析c字段字符串内容
        cJSON *pWin_speedAdress = cJSON_GetObjectItem(pJsonRoot, "win_speed"); // 解析c字段字符串内容
        cJSON *pWin_meterAdress = cJSON_GetObjectItem(pJsonRoot, "win_meter"); // 解析c字段字符串内容
        cJSON *pAirAdress = cJSON_GetObjectItem(pJsonRoot, "air"); // 解析c字段字符串内容
        cJSON *pPressureAdress = cJSON_GetObjectItem(pJsonRoot, "pressure"); // 解析c字段字符串内容
        cJSON *pHumidityAdress = cJSON_GetObjectItem(pJsonRoot, "humidity"); // 解析c字段字符串内容
        strcpy(buf->city, pCityAdress->valuestring); // 拷贝内容到字符串数组
        strcpy(buf->wea, pWeaAdress->valuestring); // 拷贝内容到字符串数组
        strcpy(buf->wea_img, pWea_imgAdress->valuestring); // 拷贝内容到字符串数组
        strcpy(buf->tem, pTemAdress->valuestring); // 拷贝内容到字符串数组
        strcpy(buf->tem_day, pTem_dayAdress->valuestring); // 拷贝内容到字符串数组
        strcpy(buf->tem_night, pTem_nightAdress->valuestring); // 拷贝内容到字符串数组
        strcpy(buf->win, pWinAdress->valuestring); // 拷贝内容到字符串数组
        strcpy(buf->win_speed, pWin_speedAdress->valuestring); // 拷贝内容到字符串数组
        strcpy(buf->win_meter, pWin_meterAdress->valuestring); // 拷贝内容到字符串数组
        strcpy(buf->air, pAirAdress->valuestring); // 拷贝内容到字符串数组
        strcpy(buf->pressure, pPressureAdress->valuestring); // 拷贝内容到字符串数组
        strcpy(buf->humidity, pHumidityAdress->valuestring); // 拷贝内容到字符串数组
    }
    else
    {
        ESP_LOGE(TAG, "解析JS失败");
    }
}

