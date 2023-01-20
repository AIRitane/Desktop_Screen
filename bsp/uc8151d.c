#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <driver/spi_master.h>

#include "uc8151d.h"

#define TAG "UC8151D"

spi_device_handle_t spi;

/*************************************管脚配置**************************************/
void screen_gpio_init()
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
void screen_spi_init(void)
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

void spi_send_cmd(const uint8_t cmd)
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

void spi_send_data(const uint8_t data)
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

void init_display()
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
        spi_send_data(buf);
    }
}

void screen_init()
{
    screen_gpio_init();
    screen_spi_init();
}

/*************************************基本UI驱动**************************************/

