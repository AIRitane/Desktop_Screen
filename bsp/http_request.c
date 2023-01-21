#include "http_request.h"

#include "esp_log.h"
#include <string.h>

#define TAG "HTTP_REQUESRT"

#define DEFAULT_PATH "/"
#define HTTP_REQUEST_RX_BUFFER_SIZE 800
#define HTTP_REQUEST_PAYLOAD_BUFFER_SIZE 256

static const char* request_payload_template = "GET %s HTTP/1.0\r\nHost: %s:%s\r\nUser-Agent:esp-idf/1.0 esp32\r\n\r\n";

int start_http_request(http_request_t *http_request)
{
    if (http_request == NULL)
    {
        ESP_LOGE(TAG, "start_http_request:null request object.");
        return ESP_FAIL;
    }

    if (http_request->host == NULL)
    {
        ESP_LOGE(TAG, "start_http_request:invalid request host");
        return ESP_FAIL;
    }

    if (http_request->port <= 0)
    {
        ESP_LOGE(TAG, "start_http_request:invalie host port");
        return ESP_FAIL;
    }

    char port_str[6];
    itoa(http_request->port, port_str, 10);

    if (http_request->web_path == NULL || strlen(http_request->web_path) == 0)
    {
        http_request->web_path = DEFAULT_PATH;
    }

    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo *res;
    int s;
    char request_payload[HTTP_REQUEST_PAYLOAD_BUFFER_SIZE];
    sprintf(request_payload, request_payload_template,
            http_request->web_path, http_request->host, port_str);

    int err = getaddrinfo(http_request->host, port_str, &hints, &res);
    if (err != 0 || res == NULL)
    {
        ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
        return ESP_FAIL;
    }

    // 创建socket
    s = socket(res->ai_family, res->ai_socktype, 0);
    if (s < 0)
    {
        ESP_LOGE(TAG, "... Failed to allocate socket.");
        freeaddrinfo(res);
        return ESP_FAIL;
    }

    // 链接
    if (connect(s, res->ai_addr, res->ai_addrlen) != 0)
    {
        ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
        close(s);
        freeaddrinfo(res);
        return ESP_FAIL;
    }
    freeaddrinfo(res);

    // 设置request
    if (write(s, request_payload, strlen(request_payload)) < 0)
    {
        ESP_LOGE(TAG, "... socket send failed");
        close(s);
        return ESP_FAIL;
    }

    // 超时设置5s 0us
    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 5;
    receiving_timeout.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                   sizeof(receiving_timeout)) < 0)
    {
        ESP_LOGE(TAG, "... failed to set socket receiving timeout");
        close(s);
        return ESP_FAIL;
    }

    // 读取
    char rx_buffer[HTTP_REQUEST_RX_BUFFER_SIZE];
    int len, ret;
    len = sizeof(rx_buffer) - 1;
    // 清空缓存
    bzero(rx_buffer, sizeof(rx_buffer));
    // 接收加密数据
    ret = read(s, (char *)rx_buffer, len);

    if (ret == -1)
    {
        ESP_LOGE(TAG, "接收Buffer读取失败");
        close(s);
        return ESP_FAIL;
    }
    else if (ret == 0)
    {
        ESP_LOGW(TAG, "接收Buffer读取为零");
        close(s);
        return ESP_FAIL;
    }
    else
    {
        http_request->data_handler(rx_buffer,len);
        rx_buffer[ret] = '\0';
        close(s);
        // ESP_LOGI(TAG, "接收到的数据：%s", rx_buffer);
        // ESP_LOGI(TAG, "接收到的数据长度：%d", ret);
    }
    return ESP_OK;
}