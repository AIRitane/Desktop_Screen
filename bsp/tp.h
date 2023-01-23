#ifndef __TP_H
#define __TP_H

#define TP_RST_PIN 5
#define TP_RST_BIT ((1ULL << TP_RST_PIN))
#define TP_INI_PIN 4
#define TP_INI_BIT ((1ULL << TP_INI_PIN))

#define I2C_SDA_PIN 33
#define I2C_SCL_PIN 32
#define I2C_FREQ_HZ 100000
#define ESP_SLAVE_ADDR 0x38
#define ACK_CHECK_EN 0x1  /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0       /*!< I2C ack value */
#define NACK_VAL 0x1

#define TP_CLICKED_DEL 5
#define TP_CLICKED_GAP 30

#include "esp_err.h"

typedef enum
{
    TP_UP = 0,
    TP_DOWEN,
    TP_LEFT,
    TP_RIGHT,
    TP_CLICKED,
    TP_LONG_CLICKED,
    TP_NONE,
} geste_e;

typedef struct
{
    uint8_t sta;
    uint8_t even;
    geste_e geste;
    uint16_t x[2]; // 最多支持5点触摸，需要使用5组坐标存储触摸点数据
    uint16_t y[2];
    uint8_t last_x;
    uint8_t last_y;
} TouchPoint_T;

void screen_tp_init();
TouchPoint_T *get_touch_point();

#endif