/*
 * https_request.c
 *
 *  Created on: 2021年7月1日
 *      Author: jenson
 */

#include "https_request.h"

#include "esp_log.h"
#include <string.h>
#include "esp_tls.h"
#include "esp_crt_bundle.h"

#define TAG "HTTPS_REQUESRT"

#define DEFAULT_PATH "/"
#define HTTPS_REQUEST_RX_BUFFER_SIZE 800
#define HTTPS_REQUEST_PAYLOAD_BUFFER_SIZE 256

static const char* request_payload_template = "GET %s HTTP/1.0\r\nHost: %s:%s\r\nUser-Agent:esp-idf/1.0 esp32\r\n\r\n";


int start_https_request(https_request_t* https_request){

	// ESP_LOGI(TAG,"start_https_request:start...");
	if(https_request == NULL){
		ESP_LOGE(TAG,"start_https_request:null request object.");
		return ESP_FAIL;
	}

	if(https_request->host == NULL){
		ESP_LOGE(TAG,"start_https_request:invalid request host");
		return ESP_FAIL;
	}

	if(https_request->port <= 0){
		ESP_LOGE(TAG,"start_https_request:invalie host port");
		return ESP_FAIL;
	}

	char port_str[6];
	itoa(https_request->port,port_str,10);

	if(https_request->web_path == NULL || strlen(https_request->web_path) == 0){
		https_request->web_path = DEFAULT_PATH;
	}

	// 配置bundle
	esp_tls_cfg_t cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

	// 创建加密连接
	struct esp_tls* tls = esp_tls_conn_http_new(https_request->host, &cfg);

	if(tls == NULL){
		ESP_LOGE(TAG,"start_https_request:can not connect to server");
		goto tls_exit;
	}

	// 构造HTTPS请求头
	char request_payload[HTTPS_REQUEST_PAYLOAD_BUFFER_SIZE];
	sprintf(request_payload,request_payload_template,
				https_request->web_path,https_request->host,port_str);
    // ESP_LOGI(TAG,"start_https_request:request payload:\n%s",request_payload);
    // ESP_LOGI(TAG,"request len:\n%d",strlen(request_payload));


    // 发送请求
    size_t written_bytes = 0;
    do {
    	 // 发送加密请求
          int ret = esp_tls_conn_write(tls,
            	request_payload + written_bytes,
                sizeof(request_payload) - written_bytes);
          if (ret >= 0) {
                // ESP_LOGI(TAG, "%d bytes written", ret);
                written_bytes += ret;
          } else if (ret != ESP_TLS_ERR_SSL_WANT_READ  && ret != ESP_TLS_ERR_SSL_WANT_WRITE) {
                ESP_LOGE(TAG, "esp_tls_conn_write  returned: [0x%02X](%s)", ret, esp_err_to_name(ret));
                goto tls_exit;
          }
     } while (written_bytes < sizeof(request_payload));

    // ESP_LOGI(TAG, "reading HTTP response...");
    char rx_buffer[HTTPS_REQUEST_RX_BUFFER_SIZE];
    int len,ret;
    do {
            len = sizeof(rx_buffer) - 1;
            // 清空缓存
            bzero(rx_buffer, sizeof(rx_buffer));
            // 接收加密数据
            ret = esp_tls_conn_read(tls, (char *)rx_buffer, len);

            if (ret == ESP_TLS_ERR_SSL_WANT_WRITE  || ret == ESP_TLS_ERR_SSL_WANT_READ) {
                continue;
            }

            if (ret < 0) {
                ESP_LOGE(TAG, "esp_tls_conn_read  returned [-0x%02X](%s)", -ret, esp_err_to_name(ret));
                break;
            }

            if (ret == 0) {
                ESP_LOGI(TAG, "connection closed");
                break;
            }

            len = ret;
            // ESP_LOGI(TAG, "%d bytes read", len);
            if(https_request->data_handler != NULL && len > 0){
                https_request->data_handler(tls,rx_buffer,len);
                if(len < HTTPS_REQUEST_RX_BUFFER_SIZE){
                	rx_buffer[len] = '\0';
                    break;
                }
             }
        } while (1);
        // ESP_LOGI(TAG,"rx_buffer = %s\n",rx_buffer);

    return ESP_OK;

tls_exit:
	esp_tls_conn_delete(tls);
	return ESP_FAIL;
}

