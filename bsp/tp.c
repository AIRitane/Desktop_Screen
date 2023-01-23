#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_timer.h"
#include "tp.h"

#define TAG "TP"
#define _abs(x) ((x > 0) ? (x) : (-x))

TouchPoint_T _gTPS;
TouchPoint_T gTPS;
// #define I2C_SDA_PIN 33
// #define I2C_SCL_PIN 32
// #define I2C_FREQ_HZ 100000
// #define ESP_SLAVE_ADDR 0x38
// #define ACK_CHECK_EN 0x1  /*!< I2C master will check ack from slave*/
// #define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
// #define ACK_VAL 0x0       /*!< I2C ack value */
// #define NACK_VAL 0x1
/******************************************I2C相关************************************************/
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = 0;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN, // select GPIO specific to your project
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SCL_PIN, // select GPIO specific to your project
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ, // select frequency specific to your project
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };

    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}

static esp_err_t i2c_master_write_slave(uint8_t u8Cmd, uint8_t *data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, u8Cmd, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_master_write_slave error\n");
    }
    return ret;
}

static esp_err_t i2c_master_set_addr(uint8_t u8Cmd)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, u8Cmd, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_master_set_addr error !! check tp is connect ?\n");
    }
    return ret;
}

static esp_err_t i2c_master_read_slave(uint8_t u8Cmd, uint8_t *data_rd, size_t size)
{
    if (size == 0)
    {
        return ESP_OK;
    }
    i2c_master_set_addr(u8Cmd);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
    for (int index = 0; index < (size - 1); index++)
    {
        i2c_master_read_byte(cmd, data_rd + index, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);

    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_master_read_slave error !! check tp is connect ?\n");
    }
    return ret;
}

/******************************************TP驱动************************************************/
// TouchPoint_T gTPS;

static void scan_ft6336()
{
    uint8_t sta = 0;
    uint8_t buf[4] = {0};
    i2c_master_read_slave(0x02, &sta, 1);

    _gTPS.sta = sta;
    // for (uint8_t i = 0; i < 2; i++)
    // {
    //     _gTPS.x[i] = 0;
    //     _gTPS.y[i] = 0;
    // }
    if (sta)
    {
        for (uint8_t i = 0; i < sta; i++)
        {
            i2c_master_read_slave(0X03 + 0x06 * i, buf, 4);
            _gTPS.x[i] = ((uint16_t)(buf[0] & 0X0F) << 8) | buf[1];
            _gTPS.y[i] = ((uint16_t)(buf[2] & 0X0F) << 8) | buf[3];
            _gTPS.even = (buf[0] & 0xc0) >> 6;
        }
    }
    else
    {
        i2c_master_read_slave(0X03, buf, 4);
        _gTPS.even = (buf[0] & 0xc0) >> 6;
    }
}

uint32_t count = 0;
uint32_t last_count = 0;
int deltax, deltay;
int32_t delta_time;
uint8_t tp_in_flag;
static void event_ft6336()
{
    // 只解析一个点的单击、双击左右滑问题
    count++;
    if (_gTPS.even == 0x02 && tp_in_flag == 0)
    {
        deltax = _gTPS.x[0];
        deltay = _gTPS.y[0];
        _gTPS.last_x = _gTPS.x[0];
        _gTPS.last_y = _gTPS.y[0];
        last_count = count;
        tp_in_flag = 1;
    }
    if (_gTPS.even == 0x01 && tp_in_flag == 1)
    {
        deltax -= _gTPS.x[0];
        deltay -= _gTPS.y[0];
        delta_time = count - last_count;
        if (_abs(deltax) < TP_CLICKED_DEL && _abs(deltay) < TP_CLICKED_DEL)
        {
            if (delta_time > TP_CLICKED_GAP)
            {
                _gTPS.geste = TP_LONG_CLICKED;
            }
            else
            {
                _gTPS.geste = TP_CLICKED;
            }
        }
        else
        {
            if (_abs(deltax) > abs(deltay))
            {
                if (deltax > 0)
                {
                    _gTPS.geste = TP_LEFT;
                }
                else
                {
                    _gTPS.geste = TP_RIGHT;
                }
            }
            else
            {
                if (deltay > 0)
                {
                    _gTPS.geste = TP_UP;
                }
                else
                {
                    _gTPS.geste = TP_DOWEN;
                }
            }
        }
        tp_in_flag = 0;
        count = 0;
    }
}

TouchPoint_T *get_touch_point()
{
    memcpy(&gTPS, &_gTPS, sizeof(gTPS));
    _gTPS.geste = TP_NONE;
    return &gTPS;
}

/******************************************定时器配置************************************************/
// #define TP_RST_PIN 5
// #define TP_RST_BIT ((1ULL << TP_RST_PIN))
// #define TP_INI_PIN 4
// #define TP_INI_BIT ((1ULL << TP_INI_PIN))

esp_timer_handle_t esp_timer_handle;

void tp_timer_callback(void *arg)
{
    scan_ft6336();
    event_ft6336();
}

static void tp_timer_init()
{
    esp_timer_create_args_t fw_timer = {
        .callback = &tp_timer_callback, // 定时器回调函数
        .arg = NULL,
        .name = "tp_timer", // 定时器名称
    };

    esp_err_t err = esp_timer_create(&fw_timer, &esp_timer_handle);
    err = esp_timer_start_periodic(esp_timer_handle, 1000 * 10); // us级定时，1000*1000=1s
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "TP定时器初始化错误");
    }
}

static void gpio_ini_rst_init()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = TP_RST_BIT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pin_bit_mask = TP_INI_BIT;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

/********************************************初始化************************************************/
void screen_tp_init()
{
    uint8_t w_data = 0;
    gpio_ini_rst_init();
    // 复位初始化
    gpio_set_level(TP_RST_PIN, 0);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level(TP_RST_PIN, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    // I2C初始化
    i2c_master_init();
    vTaskDelay(100 / portTICK_PERIOD_MS);

    w_data = 0;
    // 设置为正常操作模式
    i2c_master_write_slave(0x00, &w_data, 1);
    w_data = 22;
    // 设置触摸有效值22 越小越灵敏
    i2c_master_write_slave(0x80, &w_data, 1);
    w_data = 14;
    // 设置激活周期 不能小于12 最大14
    i2c_master_write_slave(0x88, &w_data, 1);
    w_data = 0;
    // 中断产生方式 持续电平
    i2c_master_write_slave(0xA4, &w_data, 1);

    gTPS.geste = TP_NONE;
    tp_timer_init();
}
