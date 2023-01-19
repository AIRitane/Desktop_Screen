#ifndef __HTTP_REQUEST_H
#define __HTTP_REQUEST_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

typedef void (*on_http_reqest_data_handler)(const char *data, size_t len);

typedef struct
{
    char *host;                               /*远程主机IP*/
    uint16_t port;                            /*远程主机端口*/
    char *web_path;                           /*请求路径*/
    on_http_reqest_data_handler data_handler; /*数据回调函数*/
} http_request_t;

int start_http_request(http_request_t *http_request);

#endif
