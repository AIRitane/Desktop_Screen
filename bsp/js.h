#ifndef __JS_H
#define __JS_H

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"

typedef struct
{
    char city[20];
    char wea[20];
    char wea_img[10];
    char tem[10];
    char tem_day[10];
    char tem_night[10];
    char win[20];
    char win_speed[10];
    char win_meter[10];
    char air[10];
    char pressure[10];
    char humidity[10];
}yiketianqi_t;



void one_js(char *str,char *buf);
void yiketianqi_js(char *str,yiketianqi_t *buf);
void word_js(char *str,char *buf);

#endif
