/*
 * http_request.h
 *
 *  Created on: 2021��7��1��
 *      Author: jenson
 */

#ifndef __HTTPS_REQUEST_H_
#define __HTTPS_REQUEST_H_

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
#include "esp_tls.h"
#include "esp_crt_bundle.h"

/**
 * @brief 数据回调函数
 * @brief tls 加密连接
 * @brief data 接收的数据
 * @brief len 接收数据长度
 * */
typedef void (*on_https_reqest_data_handler)(struct esp_tls *tls,const char* data,size_t len);

/**
 * @brief HTTPS请求封装
 * */
typedef struct {
	char* host; /*远程主机IP*/
	uint16_t port; /*远程主机端口*/
	char* web_path; /*请求路径*/
	on_https_reqest_data_handler data_handler; /*数据回调函数*/
}https_request_t;

/**
 * @brief 启动HTTPS请求
 * @param https_request HTTPS请求封装对象
 * */
int start_https_request(https_request_t* https_request);

#endif

