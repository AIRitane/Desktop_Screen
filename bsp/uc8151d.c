#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <driver/spi_master.h>

#include "uc8151d.h"
#include "myfont.h"

#define TAG "UC8151D"

spi_device_handle_t spi;
/*************************************管脚配置**************************************/
static void screen_gpio_init()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = PIN_CS_BIT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;

    gpio_config(&io_conf);

    io_conf.pin_bit_mask = PIN_DC_BIT;
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = PIN_RST_BIT;
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = PIN_BUSY_BIT;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

/*************************************SPI部分**************************************/
static void screen_spi_init(void)
{
    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_MISO,   // MISO信号线
        .mosi_io_num = PIN_MOSI,   // MOSI信号线
        .sclk_io_num = PIN_CLK,    // SCLK信号线
        .quadwp_io_num = -1,       // WP信号线，专用于QSPI的D2
        .quadhd_io_num = -1,       // HD信号线，专用于QSPI的D3
        .max_transfer_sz = 64 * 8, // 最大传输数据大小

    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 15 * 1000 * 1000, // Clock out at 26 MHz
        .mode = 0,                          // SPI mode 0
        .queue_size = 7,                    // We want to be able to queue 7 transactions at a time
        // .pre_cb=spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    // Initialize the SPI bus
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 0);
    ESP_ERROR_CHECK(ret);
    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}

static void spi_send_cmd(uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    gpio_set_level(PIN_DC, 0);
    gpio_set_level(PIN_CS, 0);
    memset(&t, 0, sizeof(t)); // Zero out the transaction
    // t.flags=SPI_TRANS_USE_TXDATA;
    t.length = 8;                               // Command is 8 bits
    t.tx_buffer = &cmd;                         // The data is the cmd itself
    t.user = (void *)0;                         // D/C needs to be set to 0
    ret = spi_device_polling_transmit(spi, &t); // Transmit!
    gpio_set_level(PIN_CS, 1);
    assert(ret == ESP_OK); // Should have had no issues.
}

static void spi_send_data(uint8_t data)
{
    esp_err_t ret;
    spi_transaction_t t;
    gpio_set_level(PIN_DC, 1);
    gpio_set_level(PIN_CS, 0);
    memset(&t, 0, sizeof(t));                   // Zero out the transaction
    t.length = 8;                               // Len is in bytes, transaction length is in bits.
    t.tx_buffer = &data;                        // Data
    t.user = (void *)1;                         // D/C needs to be set to 1
    ret = spi_device_polling_transmit(spi, &t); // Transmit!
    gpio_set_level(PIN_CS, 1);
    assert(ret == ESP_OK); // Should have had no issues.
}

/*************************************基础驱动**************************************/
static void lcd_chkstatus(void)
{
    int count = 0;
    unsigned char busy;
    while (1)
    {
        busy = gpio_get_level(PIN_BUSY);
        busy = (busy & 0x01);
        //=1 BUSY
        if (busy == 0)
            break;
        vTaskDelay(10 / portTICK_PERIOD_MS);
        count++;
        if (count >= 1000)
        {
            printf("---------------time out ---\n");
            break;
        }
    }
}

void deep_sleep(void) // Enter deep sleep mode
{
    spi_send_cmd(0x10); // enter deep sleep
    spi_send_data(0x01);
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

static void init_display()
{
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_RST, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    lcd_chkstatus();
    spi_send_cmd(0x12); // SWRESET
    lcd_chkstatus();

    spi_send_cmd(0x01); // Driver output control
    spi_send_data(0xC7);
    spi_send_data(0x00);
    spi_send_data(0x01);

    spi_send_cmd(0x11); // data entry mode
    spi_send_data(0x01);

    spi_send_cmd(0x44); // set Ram-X address start/end position
    spi_send_data(0x00);
    spi_send_data(0x18); // 0x0C-->(18+1)*8=200

    spi_send_cmd(0x45);  // set Ram-Y address start/end position
    spi_send_data(0xC7); // 0xC7-->(199+1)=200
    spi_send_data(0x00);
    spi_send_data(0x00);
    spi_send_data(0x00);

    spi_send_cmd(0x3C); // BorderWavefrom
    spi_send_data(0x05);

    spi_send_cmd(0x18); // Read built-in temperature sensor
    spi_send_data(0x80);

    spi_send_cmd(0x4E); // set RAM x address count to 0;
    spi_send_data(0x00);
    spi_send_cmd(0x4F); // set RAM y address count to 0X199;
    spi_send_data(0xC7);
    spi_send_data(0x00);

    vTaskDelay(100 / portTICK_PERIOD_MS);
    lcd_chkstatus();
}

void refresh(void)
{
    spi_send_cmd(0x22); // Display Update Control
    spi_send_data(0xF7);
    spi_send_cmd(0x20); // Activate Display Update Sequence
    lcd_chkstatus();
}

void screen_full_display(char *buf)
{
    init_display();
    spi_send_cmd(0x24); // write RAM for black(0)/white (1)
    for (int i = 0; i < 5000; i++)
    {
        spi_send_data(*(buf + i));
    }
    refresh(); // EPD_refresh
    deep_sleep();
}

void refresh_part(void)
{
    spi_send_cmd(0x22); // Display Update Control
    spi_send_data(0xFF);
    spi_send_cmd(0x20); // Activate Display Update Sequence
    lcd_chkstatus();
}

void screen_set_ram(char *buf)
{
    init_display();
    spi_send_cmd(0x24);
    for (int i = 0; i < 5000; i++)
    {
        spi_send_data(*(buf + i));
    }
    spi_send_cmd(0x26);
    for (int i = 0; i < 5000; i++)
    {
        spi_send_data(*(buf + i));
    }
    refresh();
    deep_sleep();
}

/*
 *PART_COLUMN   :正面看，为图片高
 *PART_LINE     :正面看，为图片宽
 */
void screen_partial_display(unsigned int x_start, unsigned int y_start, char *buf, unsigned int PART_COLUMN, unsigned int PART_LINE)
{
    unsigned int x_end, y_start1, y_start2, y_end1, y_end2;
    x_start = x_start / 8;
    x_end = x_start + PART_LINE / 8 - 1;

    y_start1 = 0;
    y_start2 = 200 - y_start;
    if (y_start >= 256)
    {
        y_start1 = y_start2 / 256;
        y_start2 = y_start2 % 256;
    }
    y_end1 = 0;
    y_end2 = y_start2 + PART_COLUMN - 1;
    if (y_end2 >= 256)
    {
        y_end1 = y_end2 / 256;
        y_end2 = y_end2 % 256;
    }

    // Add hardware reset to prevent background color change
    gpio_set_level(PIN_RST, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    // Lock the border to prevent flashing
    spi_send_cmd(0x3C); // BorderWavefrom,
    spi_send_data(0x80);

    spi_send_cmd(0x44);      // set RAM x address start/end, in page 35
    spi_send_data(x_start);  // RAM x address start at 00h;
    spi_send_data(x_end);    // RAM x address end at 0fh(15+1)*8->128
    spi_send_cmd(0x45);      // set RAM y address start/end, in page 35
    spi_send_data(y_start2); // RAM y address start at 0127h;
    spi_send_data(y_start1); // RAM y address start at 0127h;
    spi_send_data(y_end2);   // RAM y address end at 00h;
    spi_send_data(y_end1);   // ????=0

    spi_send_cmd(0x4E); // set RAM x address count to 0;
    spi_send_data(x_start);
    spi_send_cmd(0x4F); // set RAM y address count to 0X127;
    spi_send_data(y_start2);
    spi_send_data(y_start1);

    spi_send_cmd(0x24); // Write Black and White image to RAM
    for (int i = 0; i < PART_COLUMN * PART_LINE / 8; i++)
    {
        spi_send_data(*(buf + i));
    }
}

void screen_init()
{
    screen_gpio_init();
    screen_spi_init();
}

/*************************************基本UI驱动**************************************/
void screen_draw_point(char *buf, uint16_t x, uint16_t y, uint8_t color)
{
    if (x > 200 || y > 200)
    {
        ESP_LOGE(TAG, "像素超限,检查算法");
        return;
    }

    uint16_t x_i, x_o;
    x_o = x % 8;
    x_i = x / 8;

    uint16_t index;
    index = y * 25 + x_i;
    if (color)
        *(buf + index) = (*(buf + index) | (0x80 >> x_o));
    else
        *(buf + index) = (*(buf + index) & (~(0x80 >> x_o)));
}

void screen_draw_fill(char *buf, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color)
{
    for (size_t i = 0; i < x2 - x1; i++)
    {
        for (size_t j = 0; j < y2 - y1; j++)
        {
            screen_draw_point(buf, x1 + i, y1 + j, color);
        }
    }
}

void screen_draw_line(char *buf, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color)
{
    int dx = x1 - x0 >= 0 ? x1 - x0 : x0 - x1;
    int sx = x0 < x1 ? 1 : -1;
    int dy = y1 - y0 <= 0 ? y1 - y0 : y0 - y1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    if (x0 == x1 || y0 == y1)
    {
        for (int i = 0; i <= dx; i++)
        {
            for (int j = 0; j <= -dy; j++)
            {
                screen_draw_point(buf, x0 + i, y0 + j, color);
            }
        }
        return;
    }

    while ((x0 != x1) && (y0 != y1))
    {
        screen_draw_point(buf, x0, y0, color);
        if (2 * err >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (2 * err <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void screen_draw_rectangle(char *buf, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color)
{
    int min_x, min_y, max_x, max_y;
    min_x = x1 > x0 ? x0 : x1;
    max_x = x1 > x0 ? x1 : x0;
    min_y = y1 > y0 ? y0 : y1;
    max_y = y1 > y0 ? y1 : y0;
    screen_draw_line(buf, min_x, min_y, max_x, min_y, color);
    screen_draw_line(buf, min_x, max_y, max_x, max_y, color);
    screen_draw_line(buf, min_x, min_y, min_x, max_y, color);
    screen_draw_line(buf, max_x, min_y, max_x, max_y, color);
}

void screen_draw_wide_rectangle(char *buf, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t width, uint8_t color)
{
    int min_x, min_y, max_x, max_y;
    min_x = x1 > x0 ? x0 : x1;
    max_x = x1 > x0 ? x1 : x0;
    min_y = y1 > y0 ? y0 : y1;
    max_y = y1 > y0 ? y1 : y0;

    for (uint8_t i = 0; i < width; i++)
    {
        screen_draw_rectangle(buf, min_x + i, min_y + i, max_x - i, max_y - i, color);
    }
}

void screen_pic_overlay(char *buf, char *pic, uint16_t x, uint16_t y, uint16_t width, uint16_t high)
{
    if (x + width > 200 || y + high > 200)
    {
        ESP_LOGE(TAG,"图片尺寸超限");
        return;
    }
    // 宽是8的倍数，高随意
    uint16_t index;
    for (uint8_t i = 0; i < high; i++)
    {
        for (uint8_t j = 0; j < width / 8; j++)
        {
            index = j + width / 8 * i;
            for (uint8_t k = 0; k < 8; k++)
            {
                screen_draw_point(buf, x + j * 8 + k, y + i, *(pic + index) & (0x80 >> k));
            }
        }
    }
}

/*************************************汉字UI驱动**************************************/
static uint8_t UTF8toUnicode(uint8_t *ch, uint16_t *_unicode)
{
    uint8_t *p = NULL, n = 0;
    uint32_t e = 0;
    p = ch;
    if (1) // p == NULL
    {
        if (*p >= 0xfc)
        {
            /*6:<11111100>*/
            e = (p[0] & 0x01) << 30;
            e |= (p[1] & 0x3f) << 24;
            e |= (p[2] & 0x3f) << 18;
            e |= (p[3] & 0x3f) << 12;
            e |= (p[4] & 0x3f) << 6;
            e |= (p[5] & 0x3f);
            n = 6;
        }
        else if (*p >= 0xf8)
        {
            /*5:<11111000>*/
            e = (p[0] & 0x03) << 24;
            e |= (p[1] & 0x3f) << 18;
            e |= (p[2] & 0x3f) << 12;
            e |= (p[3] & 0x3f) << 6;
            e |= (p[4] & 0x3f);
            n = 5;
        }
        else if (*p >= 0xf0)
        {
            /*4:<11110000>*/
            e = (p[0] & 0x07) << 18;
            e |= (p[1] & 0x3f) << 12;
            e |= (p[2] & 0x3f) << 6;
            e |= (p[3] & 0x3f);
            n = 4;
        }
        else if (*p >= 0xe0)
        {
            /*3:<11100000>*/
            e = (p[0] & 0x0f) << 12;
            e |= (p[1] & 0x3f) << 6;
            e |= (p[2] & 0x3f);
            n = 3;
        }
        else if (*p >= 0xc0)
        {
            /*2:<11000000>*/
            e = (p[0] & 0x1f) << 6;
            e |= (p[1] & 0x3f);
            n = 2;
        }
        else
        {
            e = p[0];
            n = 1;
        }
        *_unicode = e;
    }
    return n;
}

void screen_show_chinese(char *buf, uint16_t x, uint16_t y, const char *str, uint16_t color)
{
    // 汉字三个字节
    // 复制备份
    char *stri;
    stri = (char *)malloc(strlen(str) + 1);
    memcpy(stri, str, strlen(str));
    *(stri + strlen(str)) = '\0';

    // 设置显示偏移
    uint16_t en_num = 0, ch_num = 0;
    uint16_t off_ix, off_iy;
    off_ix = x;
    off_iy = y;

    // 解析字体      横向自左到右
    uint16_t unic;
    uint8_t bitmap[87];
    uint8_t box_w = 0, box_h = 0;
    int8_t offset_x = 0, offset_y = 0;
    while (1)
    {
        if (*(stri + en_num + ch_num * 3) == '\0')
        {
            // ESP_LOGW(TAG, "OK!");
            break;
        }

        if (*(stri + en_num + ch_num * 3) == '\t')
            *(stri + en_num + ch_num * 3) = ' ';

        UTF8toUnicode((uint8_t *)(stri + en_num + ch_num * 3), &unic);
        lvgl_get_bitmap(unic, bitmap, &box_w, &box_h, &offset_x, &offset_y);

        if (off_ix + box_w + offset_x > 200 || *(stri + en_num + ch_num * 3) == '\n')
        {
            off_ix = 0;
            off_iy += FONT_LINE_HEIGHT;
        }

        off_ix += offset_x;
        uint16_t index, offset;
        uint8_t pix;
        // 只取色彩空间第一位，因此色彩空间尽可能小以换取空间
        for (uint16_t i = 0; i < box_h; i++)
        {
            for (uint16_t j = 0; j < box_w; j++)
            {
                index = i * box_w + j; //  已使用像素数量
                // offset = (index % (8 / FONT_BPP)) * FONT_BPP;
                offset = 8 - (index % (8 / FONT_BPP)) * FONT_BPP - FONT_BPP;
                pix = (*(bitmap + index / (8 / FONT_BPP))) & (0x01 << offset);
                if (color)
                    pix = pix != 0 ? 1 : 0;
                else
                    pix = pix != 0 ? 0 : 1;

                // 画该像素点
                screen_draw_point(buf, off_ix + j, off_iy + FONT_LINE_HEIGHT - offset_y - box_h + i, pix);
            }
        }
        off_ix = off_ix + FONT_GAP + box_w;
        if (*(stri + en_num + ch_num * 3) > 128)
            ch_num++;
        else
            en_num++;
    }
    free(stri);
}