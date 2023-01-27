#ifndef __API_H
#define __API_H

#include "js.h"
#include "sntp.h"

typedef enum
{
    API_TIME = 0,
    API_ONE,
    API_WEATHER,
    API_WORD
} API_TYPE_E;

extern uint8_t is_connect;

void set_api_local(char *local);
void set_api_word(char *word);
char *get_api_one();
char *get_api_word();
yiketianqi_t *get_api_weather();
sntp_time_t *get_api_time();
void send_api_event_from_isr(API_TYPE_E type);
void send_api_event(API_TYPE_E type);
uint8_t get_if_connect();
void api_init();

#endif
